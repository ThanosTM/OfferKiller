## 自旋锁与自旋锁场景

- 以==原地等待==的方式解决资源冲突，由于其忙等的特性，自旋锁不应该被长时间占有。

- 自旋锁通常十分方便，有些内核资源只锁一毫秒的时间片段，从获得自旋锁到释放不会消耗很多时间；且由于==在中断上下文中，是不允许睡眠的==，因此需要首选spinlock.


## 自旋锁的死锁与解决
自旋锁不可递归：自己等待自己获得的锁会发生死锁。

#### 场景一：内核抢占情形
1. 进程A在系统调用过程中访问了共享资源R
2. 进程B在系统调用过程中访问了共享资源R

考虑这样一种情形：A在访问共享资源R时发生了中断导致睡眠，中断唤醒了具有更高优先级的进程B，此时B试图获得自旋锁去访问共享资源R，此时将造成死锁。解决办法是：在获得自旋锁的时候，==禁止本CPU上的内核抢占==。为什么是本CPU上呢？若A和B运行在不同CPU上，A进程将在中断返回后继续执行知道释放自旋锁。

#### 场景二：中断上下文场景
1. 进程A在CPU0上运行，访问共享资源R
2. 进程B在CPU1上运行，访问共享资源R
3. 外设P的中断处理程序中访问共享资源R

考虑这样一种情形：进程A在访问共享资源R时发生了中断事件，若中断处理程序被调度到CPU1上执行并无问题，但调度到CPU0上时就会出现死锁，且进程B在CPU1上也出现死锁。解决办法是：当涉及到中断上下文访问时，==自旋锁需要禁止本CPU上的中断==。

#### 场景三：底半部中断情形
若外设P是在底半部中断中访问共享资源R，则只需要禁止底半部中断即可，禁止本地中断就有些大材小用了。

#### 场景四：不同中断之间的竞争
对于同一种中断处理程序，在单核和多核情性下都为串行执行，这是由Linux内核所确保的；如果不同中断处理程序中使用spinlock保护共享资源，则所有中断处理程序都是关闭中断的；（==？？？==）


## 自旋锁的实现
#### 数据结构
```c
typedef struct spinlock { 
        struct raw_spinlock_t rlock;  
} spinlock_t;
 
typedef struct raw_spinlock { 
    arch_spinlock_t raw_lock; 
} raw_spinlock_t;
```
这里`arch_spinlock_t`依赖体系结构实现。

这里由于RT-Linux的引入，自旋锁存在以上多种数据结构，他们之间的关系说明为：
- spinlock_t：在配置RT-Linux的PREEMPT_RT后可能会被抢占，底层的实现就不是raw_spinlock_t了，而是支持优先级翻转的mutex_t.
- raw_spinlock_t：纯正的自旋锁，不被抢占
- arch_spinlock_t：依赖体系结构的实现。

例如ARM平台现在的`arch_spinlock_t`实现：
```c
typedef struct { 
    union { 
        u32 slock; 
        struct __raw_tickets { 
            u16 owner; 
            u16 next; 
        } tickets; 
    }; 
} arch_spinlock_t;
```

需要区分引入owner和next是因为：解决多核对于自旋锁竞争不公平的现象，由于硬件cache的存在，释放spinlock的那个CPU可以更快地访问Lock，从而大大增加下一次竞争到该spinlock的机会。解决办法是引入tickets，参考饭点取号排号的模型。

#### 加锁
```c
static inline void spin_lock(spinlock_t *lock) 
{ 
    raw_spin_lock(&lock->rlock); 
}

#define raw_spin_lock(lock)    _raw_spin_lock(lock)
```

单处理器UP中：仅仅只禁止内核抢占
```c
#define _raw_spin_lock(lock)            __LOCK(lock)
 
#define __LOCK(lock) \ 
  do { preempt_disable(); ___LOCK(lock); } while (0)
```

多处理器SMP：稍复杂一些，依赖体系结构实现
```c
void __lockfunc _raw_spin_lock(raw_spinlock_t *lock) 
{ 
    __raw_spin_lock(lock); 
}
 
static inline void __raw_spin_lock(raw_spinlock_t *lock) 
{ 
    preempt_disable(); 
    spin_acquire(&lock->dep_map, 0, 0, _RET_IP_); 
    LOCK_CONTENDED(lock, do_raw_spin_trylock, do_raw_spin_lock); 
}

static inline void do_raw_spin_lock(raw_spinlock_t *lock) __acquires(lock) 
{ 
    __acquire(lock); 
    arch_spin_lock(&lock->raw_lock); 
}
```

###### 其中ARM32架构的实现方式为：
```c
static inline void arch_spin_lock(arch_spinlock_t *lock) 
{ 
    unsigned long tmp; 
    u32 newval; 
    arch_spinlock_t lockval;
 
    prefetchw(&lock->slock);－－－－－－－－－－－－－－－－－－－－－－－－（0） 
    __asm__ __volatile__( 
"1:    ldrex    %0, [%3]\n"－－－－－－－－－－－－－－－－－－－－－－－－－（1） 
"    add    %1, %0, %4\n" －－－－－－－－－－－－－－－－－－－－－－－－－－（2）
"    strex    %2, %1, [%3]\n"－－－－－－－－－－－－－－－－－－－－－－－－（3） 
"    teq    %2, #0\n"－－－－－－－－－－－－－－－－－－－－－－－－－－－－（4） 
"    bne    1b" 
    : "=&r" (lockval), "=&r" (newval), "=&r" (tmp) 
    : "r" (&lock->slock), "I" (1 << TICKET_SHIFT) 
    : "cc");
 
    while (lockval.tickets.next != lockval.tickets.owner) {－－－－－－－（5） 
        wfe();－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－（6） 
        lockval.tickets.owner = ACCESS_ONCE(lock->tickets.owner);－－－－（7） 
    }
 
    smp_mb();－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－（8） 
}
```

其中最关键的指令为ldrex和strex，这是ARM平台下多处理器核模型下的独占访问指令，其中ldrex会为本CPU核标记一个内存独占标志，strex会检查本CPU是否拥有独占标志，若拥有则进行内存更新同时返回tmp=0，并且清除所有处理器的独占标志位；若不拥有则更新失败，返回tmp=1；==所造成的效果是==：无论有多少个处理器，只有最先更新的能够保证更新成功，其余更新操作将直接失败，通常的做法是，失败的处理器重新ldrex标记独占位，直到更新成功。


###### X86的实现方式为：
lock前缀的指令，控制单元当检测到这个前缀的时候，就锁定内存总线，直到指令执行完成，在这个过程中其他处理器无法访问这个内存单元。


#### 解锁
ARM 32架构：
```c
static inline void arch_spin_unlock(arch_spinlock_t *lock)
{
  smp_mb();  
  lock->tickets.owner++; ---------------------- (0)
  dsb_sev(); ---------------------------------- (1)
}
```

执行sev指令将唤醒其他在wfe上等待的处理器。

#### 应对中断处理程序的自旋锁
正如上文中场景二所述，当位于中断上下文时，不仅仅需要禁止内核抢占，还需要关闭本地中断，这个功能由spin_lock_irqsave()和spin_unlock_irqrestore()函数提供。


## 补充：ARM结构的WFE与WFI
WFE与WFI均能让CPU进入low-power standby模式，这个模式的具体实现形式没有明确规定，在A53架构中实现为保持供电，disable时钟。

#### WFI
立即进入low-power standby 模式，直到有WFI Wakeup events发生，这个events包含IRQs与FIQ等。应用为CPUidle.

#### WFE
与WFI不同的是，执行WFE后会根据Event Reg的状态，若该寄存器为1则清零，若寄存器为0才会进入Standby模式。WFE可以被任意CPU上的SEV指令所唤醒，该指令用于修改所有CPU上的ER寄存器。一个典型的应用场景为自旋锁spinlock：
- 资源空闲
- Core1访问资源，获得spinlock，获得资源
- Core2访问资源，尝试获得spinlock失败，执行WFE指令进入low-power standby模式
- Core1访问完毕，释放spinlock，使用SEV指令唤醒Core2
- Core2获得spinlock，访问资源
## 场景
读写锁的引入是为了增加内核的并发能力，试想一下这种情况：一个内核链表元素，有很多进程都会对其进行读写，于是引入读写锁：
- 允许多个处理器并发地进行读操作，这是安全的，提高了SMP体系下的性能；
- 在写的时候，保证临界区的完全互斥；

由于读写锁的这种特性，对一个读锁的递归获取是被允许的。在中断服务程序中，如果确信对数据只有读操作的话，甚至可以使用read_lock()而不是read_lock_irqsave()即可；但对写锁还是一样，需要使用write_lock_irqsave()确保不被中断打断。


## 读写锁的实现
与spinlock一样，读写锁最底层的数据结构依赖于体系结构实现，这里仅关注ARM架构：
```c
typedef struct { 
    u32 lock; 
} arch_rwlock_t; 
```

读写锁的lock值在空闲时是0，当增加读者数量的时候，就会将其值加加，就等于读者的数量。使用了一个特殊值0x80000000代表写者，这是一个负数，可以简单地通过正负零的关系来判断是空闲、读锁、写锁。

#### arch_write_lock()
```c
static inline void arch_write_lock(arch_rwlock_t *rw)
{
	unsigned long tmp;
 
	prefetchw(&rw->lock);------------------------(0)
	__asm__ __volatile__(
"1:	ldrex	%0, [%1]\n"--------------------------(1)
"	teq	%0, #0\n"--------------------------------(2)
	WFE("ne")------------------------------------(3)
"	strexeq	%0, %2, [%1]\n"----------------------(4)
"	teq	%0, #0\n"--------------------------------(5)
"	bne	1b"--------------------------------------(6)
	: "=&r" (tmp)
	: "r" (&rw->lock), "r" (0x80000000)
	: "cc");
 
	smp_mb();------------------------------------(7)
}
```
0. 硬件preloading cache，提升性能；
1. ldrex标记独占，取出tmp = rw->lock的值；
2. 判断是否tmp是否等于0（是否空闲）
3. 若不等于0将通过WFE指令CPU进入等待期；
4. 若等于0则将0x80000000赋给rw->lock；
5. 若strex成功，则说明独占访问成功
6. 若失败，则需要重新ldrex
7. 内存屏障，保证执行顺序；

#### arch_write_unlock()
```c
static inline void arch_write_unlock(arch_rwlock_t *rw) 
{ 
    smp_mb(); ---------------------------（0）
 
    __asm__ __volatile__( 
    "str    %1, [%0]\n" -----------------（1）
    : 
    : "r" (&rw->lock), "r" (0)
    : "cc");
 
    dsb_sev(); --------------------------（2）
}
```

#### arch_read_lock()
```c
static inline void arch_read_lock(arch_rwlock_t *rw)
{
	unsigned long tmp, tmp2;
 
	prefetchw(&rw->lock);
	__asm__ __volatile__(
"1:	ldrex	%0, [%2]\n" ----------- （0）
"	adds	%0, %0, #1\n" --------- （1）
"	strexpl	%1, %0, [%2]\n" ------- （2）
	WFE("mi") --------------------- （3）
"	rsbpls	%0, %1, #0\n" --------- （4）
"	bmi	1b" ----------------------- （5）
	: "=&r" (tmp), "=&r" (tmp2)
	: "r" (&rw->lock)
	: "cc");
 
	smp_mb();
}
```

#### arch_read_unlock()
```c
static inline void arch_read_lock(arch_rwlock_t *rw)
{
	unsigned long tmp, tmp2;
 
	prefetchw(&rw->lock);
	__asm__ __volatile__(
"1:	ldrex	%0, [%2]\n" ----------- （0）
"	adds	%0, %0, #1\n" --------- （1）
"	strexpl	%1, %0, [%2]\n" ------- （2）
	WFE("mi") --------------------- （3）
"	rsbpls	%0, %1, #0\n" --------- （4）
"	bmi	1b" ----------------------- （5）
	: "=&r" (tmp), "=&r" (tmp2)
	: "r" (&rw->lock)
	: "cc");
 
	smp_mb();
}
```
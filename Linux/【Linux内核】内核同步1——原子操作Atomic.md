## 原子操作
Linux若干不可打断的操作，将整型原子操作定义为结构体.

#### 数据结构
```c
typedef struct {
	int counter;
} atomic_t;
```

#### 基本操作
一些例子：
```c
atomic_inc(v); // 原子变量自增1
atomic_dec(v); // 原子变量自减1
atomic_read(v) // 读取一个原子量
atomic_add(int i, atomic_t *v) // 原子量增加 i
atomic_sub(int i, atomic_t *v) // 原子量减少 i
```

真正的操作依赖于体系结构：
```c
static inline void atomic_inc(atomic_t *v)
{
    atomic_add_return(1, v);
}
```

#### 底层实现
ARM架构的实现为：
```c
#define atomic_inc_return(v)	atomic_add_return(1, (v))
static inline int atomic_add_return(int i, atomic_t *v)
{
	unsigned long tmp;
	int result;
	smp_mb();
 
	__asm__ __volatile__("@ atomic_add_return\n"
		"1:	ldrex	%0, [%3]\n"              /*【1】独占方式加载v->counter到result*/
		"	add	%0, %0, %4\n"                /*【2】result加一*/
		"	strex	%1, %0, [%3]\n"          /*【3】独占方式将result值写回v->counter*/
		"	teq	%1, #0\n"                    /*【4】判断strex更新内存是否成*/
		"	bne	1b"                          /*【5】不成功跳转到1:*/
		: "=&r" (result), "=&r" (tmp), "+Qo" (v->counter)   /*输出部*/
		: "r" (&v->counter), "Ir" (i)                       /*输入部*/
		: "cc");                                           /*损坏部*/
 
	smp_mb();
	return result;
}
```

最核心的是两条汇编指令ldrex和strex.

#### ldrex：读取内存中的值，并标记为独占访问
```
LDREX Rx, [Ry]
```
读取寄存器Ry指向的4字节内存值，并将其保存到Rx寄存器中，同时标记对这段内存区域的独占访问；若已经被标记为独占访问了，也不会对指令的执行产生影响。

#### strex：在更新内存数值时，检查该段内存是否为独占访问，并以此来决定是否更新
```
STREX Rx, Ry, [Rz]
```
- 若Rz指向的内存段被标记为独占访问了，则将寄存器Ry的值更新到Rz指向的内存段中，并将Rx设为0，并将独占访问标记位清除
- 若没有设置独占位，则不会更新内存，将Rx设为1

#### 独占监视器Exclusive Monitor
ARM架构提供两类监视器，本地监视器与全局监视器，如果要对非共享内存区中的值进行独占访问，只需要涉及本处理器内部的本地监视器就可以了；而如果要对共享内存区中的内存进行独占访问，除了要涉及到本处理器内部的本地监视器外，由于该内存区域可以被系统中所有处理器访问到，因此还必须要由全局监视器来协调。下面仅考虑多核对共享内存的独占访问：
- CPU2上的线程3最早执行LDREX，标记独占位，更新本地监视器与全局监视器；
- CPU1上的线程1执行LDREX，更新本地与全局监视器，这时在全局监视器上，CPU1和CPU2都对该段内存进行了独占标记；
- CPU1上的线程2执行LDREX，本地与全局监视器均对这段内存有独占标记，不会影响执行；
- CPU1的线程1执行STREX指令，发现本地和全局监视器都对该段内存有独占标记，因此更新成功，并清除本地与全局监视器上的独占标记；
- CPU2上的线程3执行STREX指令：本地拥有独占标记，但全局的已被清除，更新失败；
- CPU1上的线程2执行STREX指令，本地和全局均无独占标记，更新失败。

**这套机制的精髓就是，无论有多少个处理器，有多少个地方会申请对同一个内存段进行操作，保证只有最早的更新可以成功，这之后的更新都会失败。失败了就证明对该段内存有访问冲突了。实际的使用中，可以重新用LDREX读取该段内存中保存的最新值，再处理一次，再尝试保存，直到成功为止。**
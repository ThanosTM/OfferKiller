## 信号量机制
#### 简述
信号量与自旋锁不同的是，它是一种睡眠锁，特点是：如果有一个任务试图获取一个已被占用的信号量，将会被推入等待队列，让其进行睡眠；此刻处理器重新选择一个进程并调度执行，当持有的信号量被释放，处于等待队列的进程被唤醒，此时它可以获取到该信号量。

#### 特性
- 信号量不适合短时间持有，因为会导致进程睡眠，维护队列、唤醒、切换等时间开销比忙等的效率低；
- 由于其睡眠特性，只能在进程上下文中使用，无法在中断上下文中使用信号量；

#### 与自旋锁的选择
- 中断上下文中只能使用自旋锁；
- 既可以选择自旋锁又能选择信号量时，短时长使用自旋锁，长时间使用信号量；
- 信号量允许多个锁的持有者，而自旋锁最多只有一个持锁者；特别的，对于最多只有一个持锁者的信号量也称互斥量；


## 信号量的实现机制
#### 数据结构
```c
struct semaphore {
	raw_spinlock_t		lock;
	unsigned int		count;
	struct list_head	wait_list;
};
```
- 信号量包含一个count表示资源数，大于0表示资源空闲，并且最多还剩多少资源可以用；等于0说明信号量忙，但暂时没有进程等待；小于0说明资源不可用，并至少有一个进程在等待。
- 此外，信号量使用spinlock保护其计数器与等待队列，wait_list存放等待队列链表的地址。

#### 实现函数
Linux提供了多种down()函数：
- down()
- down_interruptible()
- down_timeout()
- down_trylock()
这些函数均调用到__down_common()函数完成获取信号量的逻辑；其中down()将等待的资源设为uninterruptible，不常用；down_interruptible()在设备驱动函数中使用的比较多，与down()不同的是，若睡眠的进程在获得等待的资源之前被另外一个信号唤醒，则将返回-EINTR，这样设备驱动函数可以放弃I/O操作。

例如down_interruptible()函数的实现为：
```c
static noinline int __sched __down_interruptible(struct semaphore *sem)
{
	return __down_common(sem, TASK_INTERRUPTIBLE, MAX_SCHEDULE_TIMEOUT);
}
 
int down_interruptible(struct semaphore *sem)
{
	unsigned long flags;
	int result = 0;
 
	raw_spin_lock_irqsave(&sem->lock, flags);
	if (likely(sem->count > 0))
		sem->count--;
	else
		result = __down_interruptible(sem);
	raw_spin_unlock_irqrestore(&sem->lock, flags);
 
	return result;
}
```

#### __down_common()函数
```c
/*
 * Because this function is inlined, the 'state' parameter will be
 * constant, and thus optimised away by the compiler.  Likewise the
 * 'timeout' parameter for the cases without timeouts.
 */
static inline int __sched __down_common(struct semaphore *sem, long state,
								long timeout)
{
	struct task_struct *task = current;
	struct semaphore_waiter waiter;

	list_add_tail(&waiter.list, &sem->wait_list);
	waiter.task = task;
	waiter.up = 0;

	for (;;) {
		if (signal_pending_state(state, task))
			goto interrupted;
		if (timeout <= 0)
			goto timed_out;
		__set_task_state(task, state);
		spin_unlock_irq(&sem->lock);
		timeout = schedule_timeout(timeout);
		spin_lock_irq(&sem->lock);
		if (waiter.up)
			return 0;
	}

 timed_out:
	list_del(&waiter.list);
	return -ETIME;

 interrupted:
	list_del(&waiter.list);
	return -EINTR;
}
```

#### up()函数
```c
void up(struct semaphore *sem)
{
	unsigned long flags;
 
	raw_spin_lock_irqsave(&sem->lock, flags);
	if (likely(list_empty(&sem->wait_list)))
		sem->count++;
	else
		__up(sem);
	raw_spin_unlock_irqrestore(&sem->lock, flags);
}
```

```c
static noinline void __sched __up(struct semaphore *sem)
{
	struct semaphore_waiter *waiter = list_first_entry(&sem->wait_list,
						struct semaphore_waiter, list);
	list_del(&waiter->list);
	waiter->up = true;
	wake_up_process(waiter->task);
}
```


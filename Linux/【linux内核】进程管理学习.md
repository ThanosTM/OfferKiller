## 进程控制块PCB
一个进程封装的资源包含：内存、文件、文件系统、信号、控制台等等，因此需要维护一个数据结构PCB来描述一个进程，也就是`struct task_struct`，定义于include/linux/sched.h中
```c
struct task_struct {
	volatile long state;	/* -1 unrunnable, 0 runnable, >0 stopped */
	void *stack;
	atomic_t usage;
	unsigned int flags;	/* per process flags, defined below */
	unsigned int ptrace;

	int lock_depth;		/* BKL lock depth */

#ifdef CONFIG_SMP
#ifdef __ARCH_WANT_UNLOCKED_CTXSW
	int oncpu;
#endif
#endif

	int prio, static_prio, normal_prio;
	unsigned int rt_priority;
	const struct sched_class *sched_class;
	struct sched_entity se;
	struct sched_rt_entity rt;

#ifdef CONFIG_PREEMPT_NOTIFIERS
	/* list of struct preempt_notifier: */
	struct hlist_head preempt_notifiers;
#endif

	/*
	 * fpu_counter contains the number of consecutive context switches
	 * that the FPU is used. If this is over a threshold, the lazy fpu
	 * saving becomes unlazy to save the trap. This is an unsigned char
	 * so that after 256 times the counter wraps and the behavior turns
	 * lazy again; this to deal with bursty apps that only use FPU for
	 * a short time
	 */
	unsigned char fpu_counter;
#ifdef CONFIG_BLK_DEV_IO_TRACE
	unsigned int btrace_seq;
#endif

	unsigned int policy;
	cpumask_t cpus_allowed;

#ifdef CONFIG_TREE_PREEMPT_RCU
	int rcu_read_lock_nesting;
	char rcu_read_unlock_special;
	struct rcu_node *rcu_blocked_node;
	struct list_head rcu_node_entry;
#endif /* #ifdef CONFIG_TREE_PREEMPT_RCU */

#if defined(CONFIG_SCHEDSTATS) || defined(CONFIG_TASK_DELAY_ACCT)
	struct sched_info sched_info;
#endif

	struct list_head tasks;
	struct plist_node pushable_tasks;

	struct mm_struct *mm, *active_mm;
#if defined(SPLIT_RSS_COUNTING)
	struct task_rss_stat	rss_stat;
#endif
/* task state */
	int exit_state;
	int exit_code, exit_signal;
	int pdeath_signal;  /*  The signal sent when the parent dies  */
	/* ??? */
	unsigned int personality;
	unsigned did_exec:1;
	unsigned in_execve:1;	/* Tell the LSMs that the process is doing an
				 * execve */
	unsigned in_iowait:1;


	/* Revert to default priority/policy when forking */
	unsigned sched_reset_on_fork:1;

	pid_t pid;
	pid_t tgid;

	/* 
	 * pointers to (original) parent process, youngest child, younger sibling,
	 * older sibling, respectively.  (p->father can be replaced with 
	 * p->real_parent->pid)
	 */
	struct task_struct *real_parent; /* real parent process */
	struct task_struct *parent; /* recipient of SIGCHLD, wait4() reports */
	/*
	 * children/sibling forms the list of my natural children
	 */
	struct list_head children;	/* list of my children */
	struct list_head sibling;	/* linkage in my parent's children list */
	struct task_struct *group_leader;	/* threadgroup leader */

	/*
	 * This is the tracer handle for the ptrace BTS extension.
	 * This field actually belongs to the ptracer task.
	 */
	struct bts_context *bts;

	/* PID/PID hash table linkage. */
	struct pid_link pids[PIDTYPE_MAX];
	struct list_head thread_group;

	struct completion *vfork_done;		/* for vfork() */
	int __user *set_child_tid;		/* CLONE_CHILD_SETTID */
	int __user *clear_child_tid;		/* CLONE_CHILD_CLEARTID */

	cputime_t utime, stime, utimescaled, stimescaled;
	cputime_t gtime;
#ifndef CONFIG_VIRT_CPU_ACCOUNTING
	cputime_t prev_utime, prev_stime;
#endif
	unsigned long nvcsw, nivcsw; /* context switch counts */
	struct timespec start_time; 		/* monotonic time */
	struct timespec real_start_time;	/* boot based time */
/* mm fault and swap info: this can arguably be seen as either mm-specific or thread-specific */
	unsigned long min_flt, maj_flt;

	struct task_cputime cputime_expires;
	struct list_head cpu_timers[3];

/* process credentials */
	const struct cred *real_cred;	/* objective and real subjective task
					 * credentials (COW) */
	const struct cred *cred;	/* effective (overridable) subjective task
					 * credentials (COW) */
	struct mutex cred_guard_mutex;	/* guard against foreign influences on
					 * credential calculations
					 * (notably. ptrace) */
	struct cred *replacement_session_keyring; /* for KEYCTL_SESSION_TO_PARENT */

	char comm[TASK_COMM_LEN]; /* executable name excluding path
				     - access with [gs]et_task_comm (which lock
				       it with task_lock())
				     - initialized normally by setup_new_exec */
/* file system info */
	int link_count, total_link_count;
#ifdef CONFIG_SYSVIPC
/* ipc stuff */
	struct sysv_sem sysvsem;
#endif
#ifdef CONFIG_DETECT_HUNG_TASK
/* hung task detection */
	unsigned long last_switch_count;
#endif
/* CPU-specific state of this task */
	struct thread_struct thread;
/* filesystem information */
	struct fs_struct *fs;
/* open file information */
	struct files_struct *files;
/* namespaces */
	struct nsproxy *nsproxy;
/* signal handlers */
	struct signal_struct *signal;
	struct sighand_struct *sighand;

	sigset_t blocked, real_blocked;
	sigset_t saved_sigmask;	/* restored if set_restore_sigmask() was used */
	struct sigpending pending;

	...
};
```

一张图描述一下：
![image](https://blog.acean.vip/Linux%20Kernel/_image/2018-05-31-21-38-24.jpg)


#### `task_struct`之间的关系
在Linux中，并行地使用了三种数据结构将task_struct组织起来，这是一种空间换时间的思想（其他管理中也大量使用）：
- 双向链表：
```c
struct list_head tasks;
```
这个类型的prev和next字段分别指向前面和后面的task_struct元素

- 树：用于描述进程之间的父子关系，包含real_parent、parent、children、sibling，其中real_parent和parent的值大多数情况相同，在涉及ptrace时也可不同。可以使用==pstree指令==打印出Linux进程的树形结构。

- 哈希表：Linux使用pid映射到task_struct，这是为了根据pid号对进程进行快速检索。


#### 进程状态
![image](https://blog.acean.vip/Linux%20Kernel/_image/2018-06-01-10-01-23.jpg)

- 就绪and运行TASK_RUNNING：正在CPU上执行或准备执行，放在Linux内核CPU runqueue队列中但还未被调度到的就是这个状态；
- 可中断的等待状态、浅睡眠TASK_INTERRUPTIBLE：资源到位或者信号打断，都能够使该进程唤醒至就绪；
- 不可中断的等待状态、深睡眠TASK_UNINTERRUPTIBLE：使用场景，当进程打开一个设备文件，相应设备驱动程序开始探测硬件设备，在探测完成之前设备驱动程序不能被中断，否则硬件设备会处于不可预知的状态；
- 停止TASK_STOPPED：接收到SIGSTOP信号后暂停执行，也可以在ptrace调试时暂停指定，后可以再转为就绪态；
- 僵死状态EXIT_ZOMBIE：进程终止执行，还未返回有关死亡进程的信息，这时task_struct的资源已经被内核释放，只剩下一具空壳用以告知父进程死亡信息，直到父进程发出wait4()或waitpid()真正被杀死。


## 进程创建
#### clone()：Linux轻量级进程的创建
该函数有一个很重要的参数就是flags，其中可以设置子进程共享父进程的哪些资源，例如：
- CLONE_VM：共享内存描述符和所有的页表
- CLONE_VFORK：发出vfork()系统调用时设置
- CLONE_FILES：共享打开的文件表

#### fork()
使用clone()实现，其中清除了所有的标志，即创建一个全新的进程，父子进程只是暂时地共享同一个资源，其中任一进程试图修改则将执行写时复制COW。
![image](https://blog.acean.vip/Linux%20Kernel/_image/2018-06-01-14-51-24.jpg)

#### vfork()
使用clone()实现，设置了CLONE_VM|CLONE_VFORK标志位，因此最大的特点是，内存资源是共享的。此外还有一个特点是：vfork会阻塞住父进程，直到子进程退出、或者子进程调用了exec函数族，这在其他学习文档中有描述过。

#### 线程
在Linux中，使用标准POSIX接口pthread_create()来创建线程，实际上该函数也将会调用clone()函数，只是传入的参数为：`CLONE_VM|CLONE_FS|CLONE_FILES|CLONE_SIGHAND|CLONE_THREAD|...`等，表明共享父进程的所有资源。

#### POSIX对进程和线程的要求
![image](https://blog.acean.vip/Linux%20Kernel/_image/2018-06-01-16-16-28.jpg)

POSIX标准要求同一个进程内的多个线程应该具有同一个pid，但是在Linux中，进程线程实际上都是由task_struct来描述的，具有不同的pid.对此，Linux使用了一个障眼法，在task_struct中增加了tgid字段，使其等于该线程组group leader的pid值，然后getpid()返回的实际上是tgid的值。

#### `do_fork()`函数分析
![image](https://img-blog.csdn.net/20160602203201604)
```c
/*
 *  Ok, this is the main fork-routine.
 *
 * It copies the process, and if successful kick-starts
 * it and waits for it to finish using the VM if required.
 */
long do_fork(unsigned long clone_flags,
	      unsigned long stack_start,
	      struct pt_regs *regs,
	      unsigned long stack_size,
	      int __user *parent_tidptr,
	      int __user *child_tidptr)
{
	struct task_struct *p;
	int trace = 0;
	long nr;

	/*
	 * Do some preliminary argument and permissions checking before we
	 * actually start allocating stuff
	 */
	if (clone_flags & CLONE_NEWUSER) {
		if (clone_flags & CLONE_THREAD)
			return -EINVAL;
		/* hopefully this check will go away when userns support is
		 * complete
		 */
		if (!capable(CAP_SYS_ADMIN) || !capable(CAP_SETUID) ||
				!capable(CAP_SETGID))
			return -EPERM;
	}

	/*
	 * We hope to recycle these flags after 2.6.26
	 */
	if (unlikely(clone_flags & CLONE_STOPPED)) {
		static int __read_mostly count = 100;

		if (count > 0 && printk_ratelimit()) {
			char comm[TASK_COMM_LEN];

			count--;
			printk(KERN_INFO "fork(): process `%s' used deprecated "
					"clone flags 0x%lx\n",
				get_task_comm(comm, current),
				clone_flags & CLONE_STOPPED);
		}
	}

	/*
	 * When called from kernel_thread, don't do user tracing stuff.
	 */
	if (likely(user_mode(regs)))
		trace = tracehook_prepare_clone(clone_flags);

	p = copy_process(clone_flags, stack_start, regs, stack_size,
			 child_tidptr, NULL, trace);
	/*
	 * Do this prior waking up the new thread - the thread pointer
	 * might get invalid after that point, if the thread exits quickly.
	 */
	if (!IS_ERR(p)) {
		struct completion vfork;

		trace_sched_process_fork(current, p);

		nr = task_pid_vnr(p);

		if (clone_flags & CLONE_PARENT_SETTID)
			put_user(nr, parent_tidptr);

		if (clone_flags & CLONE_VFORK) {
			p->vfork_done = &vfork;
			init_completion(&vfork);
		}

		audit_finish_fork(p);
		tracehook_report_clone(regs, clone_flags, nr, p);

		/*
		 * We set PF_STARTING at creation in case tracing wants to
		 * use this to distinguish a fully live task from one that
		 * hasn't gotten to tracehook_report_clone() yet.  Now we
		 * clear it and set the child going.
		 */
		p->flags &= ~PF_STARTING;

		if (unlikely(clone_flags & CLONE_STOPPED)) {
			/*
			 * We'll start up with an immediate SIGSTOP.
			 */
			sigaddset(&p->pending.signal, SIGSTOP);
			set_tsk_thread_flag(p, TIF_SIGPENDING);
			__set_task_state(p, TASK_STOPPED);
		} else {
			wake_up_new_task(p, clone_flags);
		}

		tracehook_report_clone_complete(trace, regs,
						clone_flags, nr, p);

		if (clone_flags & CLONE_VFORK) {
			freezer_do_not_count();
			wait_for_completion(&vfork);
			freezer_count();
			tracehook_report_vfork_done(p, nr);
		}
	} else {
		nr = PTR_ERR(p);
	}
	return nr;
}
```

流程：
- 调用`copy_process()`为子进程复制一份进程信息；
- 调用`wake_up_new_task()`将子进程加入调度器，为之分配CPU
- 如果设置了vfork，则父进程等待子进程完成exec()替换自己的地址空间；

#### `copy_process()`函数分析
![image](https://img-blog.csdn.net/20160602203329356)
流程：
- `dup_task_struct`：复制当前task_struct；
- 初始化一些自旋锁、挂起信号、CPU定时器等；
- 调用`sched_fork()`初始化进程数据结构，并把进程状态设置为TASK_RUNNING；
- 复制所有进程信息：文件系统、信号处理函数、信号、内存管理等等；
```c
/* copy all the process information */
if ((retval = copy_semundo(clone_flags, p)))
	goto bad_fork_cleanup_audit;
if ((retval = copy_files(clone_flags, p)))
	goto bad_fork_cleanup_semundo;
if ((retval = copy_fs(clone_flags, p)))
	goto bad_fork_cleanup_files;
if ((retval = copy_sighand(clone_flags, p)))
	goto bad_fork_cleanup_fs;
if ((retval = copy_signal(clone_flags, p)))
	goto bad_fork_cleanup_sighand;
if ((retval = copy_mm(clone_flags, p)))
	goto bad_fork_cleanup_signal;
if ((retval = copy_namespaces(clone_flags, p)))
	goto bad_fork_cleanup_mm;
if ((retval = copy_io(clone_flags, p)))
	goto bad_fork_cleanup_namespaces;
```

- 初始化子进程的内核栈
```c
retval = copy_thread(clone_flags, stack_start, stack_size, p, regs);
if (retval)
	goto bad_fork_cleanup_io;
```

这里copy_thread的实现是依赖具体的体系结构的，因为需要构造出所有寄存器的值，包含内核在进程之间切换时需要保存和恢复的进程信息。

- 为新进程分配pid


## 进程切换
#### 用户抢占
Linux用户态下触发抢占的情况很多，例如：等待在某些资源上被唤醒时、进程优先级被改变时、进程时间片被耗尽时等等，但不会立即触发抢占，而是通过设置一个抢占标志位`TIF_NEED_RESCHED`，等到达一个抢占点的时候才会被抢占。

- 标记抢占位的函数：
![image](https://img2018.cnblogs.com/blog/1771657/202002/1771657-20200229212204811-1734127314.png)
- 抢占的执行点：
![image](https://img2018.cnblogs.com/blog/1771657/202002/1771657-20200229212235516-1038164597.png)
返回用户空间时会对标志位进行判断，如果设置了`TIF_NEED_RESCHED`则将进行调度切换。

#### 内核抢占
Linux内核抢占的3种模型：
- `CONFIG_PREEMPT_NONE`：不支持内核抢占，中断退出后需要等到低优先级任务主动让出CPU时才会发生切换；
- `CONFIG_PREEMPT_VOLUNTARY`：自愿抢占，代码中增加抢占点，高优先级任务可以在抢占点处进行切换；
- `CONFIG_PREEMPT`：立即抢占，中断退出后遇到高优先级任务可以立即发生抢占；

![image](https://img2018.cnblogs.com/blog/1771657/202002/1771657-20200229212309761-334760914.png)


#### 切换入口`schedule()`
![image](https://img2018.cnblogs.com/blog/1771657/202002/1771657-20200229212346352-1357028476.png)

```c
/*
 * schedule() is the main scheduler function.
 */
asmlinkage void __sched schedule(void)
{
	struct task_struct *prev, *next;
	unsigned long *switch_count;
	struct rq *rq;
	int cpu;

need_resched:
	preempt_disable();
	cpu = smp_processor_id();
	rq = cpu_rq(cpu);
	rcu_sched_qs(cpu);
	prev = rq->curr;
	switch_count = &prev->nivcsw;

	release_kernel_lock(prev);
need_resched_nonpreemptible:

	schedule_debug(prev);

	if (sched_feat(HRTICK))
		hrtick_clear(rq);

	raw_spin_lock_irq(&rq->lock);
	update_rq_clock(rq);
	clear_tsk_need_resched(prev);

	if (prev->state && !(preempt_count() & PREEMPT_ACTIVE)) {
		if (unlikely(signal_pending_state(prev->state, prev)))
			prev->state = TASK_RUNNING;
		else
			deactivate_task(rq, prev, 1);
		switch_count = &prev->nvcsw;
	}

	pre_schedule(rq, prev);

	if (unlikely(!rq->nr_running))
		idle_balance(cpu, rq);

	put_prev_task(rq, prev);
	next = pick_next_task(rq);

	if (likely(prev != next)) {
		sched_info_switch(prev, next);
		perf_event_task_sched_out(prev, next);

		rq->nr_switches++;
		rq->curr = next;
		++*switch_count;

		context_switch(rq, prev, next); /* unlocks the rq */
		/*
		 * the context switch might have flipped the stack from under
		 * us, hence refresh the local variables.
		 */
		cpu = smp_processor_id();
		rq = cpu_rq(cpu);
	} else
		raw_spin_unlock_irq(&rq->lock);

	post_schedule(rq);

	if (unlikely(reacquire_kernel_lock(current) < 0)) {
		prev = rq->curr;
		switch_count = &prev->nivcsw;
		goto need_resched_nonpreemptible;
	}

	preempt_enable_no_resched();
	if (need_resched())
		goto need_resched;
}
```

主要的逻辑：
- 获得当前运行的CPU获取运行队列，得到当前的task；
- 根据调度类来选择切换的下一个task，`pick_next_task()`，这里每个调度器类实现了自己的`pick_next_task()`逻辑，例如CFS完全公平调度，这里使用到了函数指针做分支。
- `context_switch()`完成切换；

#### 上下文切换`context_switch()`
![image](https://img2018.cnblogs.com/blog/1771657/202002/1771657-20200229212409378-167088356.png)

```c
/*
 * context_switch - switch to the new MM and the new
 * thread's register state.
 */
static inline void
context_switch(struct rq *rq, struct task_struct *prev,
	       struct task_struct *next)
{
	struct mm_struct *mm, *oldmm;

	prepare_task_switch(rq, prev, next);
	trace_sched_switch(rq, prev, next);
	mm = next->mm;
	oldmm = prev->active_mm;
	/*
	 * For paravirt, this is coupled with an exit in switch_to to
	 * combine the page table reload and the switch backend into
	 * one hypercall.
	 */
	arch_start_context_switch(prev);

	if (likely(!mm)) {
		next->active_mm = oldmm;
		atomic_inc(&oldmm->mm_count);
		enter_lazy_tlb(oldmm, next);
	} else
		switch_mm(oldmm, mm, next);

	if (likely(!prev->mm)) {
		prev->active_mm = NULL;
		rq->prev_mm = oldmm;
	}
	/*
	 * Since the runqueue lock will be released by the next
	 * task (which is an invalid locking op but in the case
	 * of the scheduler it's an obvious special-case), so we
	 * do an early lockdep release here:
	 */
#ifndef __ARCH_WANT_UNLOCKED_CTXSW
	spin_release(&rq->lock.dep_map, 1, _THIS_IP_);
#endif

	/* Here we just switch the register state and the stack. */
	switch_to(prev, next, prev);

	barrier();
	/*
	 * this_rq must be evaluated again because prev may have moved
	 * CPUs since it called schedule(), thus the 'rq' on its stack
	 * frame will be invalid.
	 */
	finish_task_switch(this_rq(), prev);
}
```

核心的逻辑是两部分：
- 进程地址空间的切换，这里就需要判断是内核进程还是用户进程，所有用户进程共用同一个内核地址空间而拥有不同的用户地址空间；而内核进程没有用户地址空间。这其中涉及到页表的操作和cache/tlb的刷新操作。
- 寄存器的切换；

==进程切换的开销不仅仅是页表、硬件上下文的切换开销，还包含了cache/tlb刷新后带来的miss开销。==


## 内核线程
Linux把一些任务委托给内核线程，例如刷新磁盘高速缓存、交换出不用的页框、维护网络连接等，这些线程==只运行在内核态==，也从来==不会访问属于用户态的虚拟地址空间==，因此不必受到不必要的用户态上下文切换拖累。上文`context_switch()`中，判断为内核线程时就不需要进行页表切换。

#### 创建内核线程
`kernel_thread()`函数用于创建一个新的内核线程，该函数本质上使用以下方式调用`do_fork()`：
```c
    do_fork(flags|CLONE_VM|UNTRACED, ...);
```
由于内核线程无论如何不会访问用户态地址空间，因此设置CLONE_VM避免复制调用进程的页表。

#### 进程0
- 进程0是所有进程的祖先，也叫idle进程，swapper进程。该进程使用==静态分配的数据结构==，而其他进程都是动态创建的。
- 在`start_kernel()`中初始化内核所需要的所有数据结构，激活中断，创建另一个叫进程1的内核进程（init进程）：
```c
kernel_thread(init, NULL, CLONE_FS|CLONE_SIGHAND);
```
该内核进程PID为1，并与进程0共享每进程所有的内核数据结构。
- 创建init后，进程0执行`cpu_idle()`函数，该函数本质上是执行hlt汇编指令，只有当没有其他进程处于就绪态时才会被选择执行到。
- 多处理器系统中，每一个CPU都有一个进程0，启动过程是这样的：开机BIOS启动某一个CPU，并禁用其他CPU，这个CPU 0的进程0初始化内核数据结构，并启用其他CPU，并通过`copy_process()`函数创建另外的swapper进程。

#### 进程1
`init()`函数调用系统调用`execve()`装入可执行程序init，依次完成内核初始化，init内核线程变为一个普通进程。在系统关闭之前，init进程一直存活，因为它创建和监控在操作系统外层执行的所有进程活动。

#### 其他内核线程
- keventd：事件，执行`keventd_wq`工作队列
- kapmd：处理与高级电源管理APM相关的事件
- kswapd：执行内存回收
- pdflush：刷新脏缓冲区中的内容到磁盘以回收内存
- kblockd：执行`kblockd_workqueue`工作队列中的函数，周期性地激活块设备驱动程序
- ksoftirqd：运行tasklet，系统中每CPU都有这样的内核线程
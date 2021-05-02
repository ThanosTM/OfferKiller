## Linux 0.11的调度算法分析
![image](https://img-blog.csdnimg.cn/20200315185356297.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L20wXzM3NTY4NTIx,size_16,color_FFFFFF,t_70)
- 简单实用，不区分实时进程和普通进程，所有进程采用同一套调度策略
- 每次遍历所有进程，复杂度自然为O(N)
- 动态优先级思想，综合了剩余时间片和静态优先级，偏向于前台I/O密集型任务，当阻塞的进程就绪时，一般counter较大，更容易得到调度。

#### 参考：
- MOOC-哈工大-李治军-操作系统课程
- 赵炯-Linux内核完全注释

## O(N)调度算法的总结
![image](https://img-blog.csdnimg.cn/20200315185155537.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L20wXzM3NTY4NTIx,size_16,color_FFFFFF,t_70)
- 随着进程数量的增多，该算法的时间复杂度变得很高
- 在SMP环境下仍然维持一个runqueue，进程需要在多个CPU之间来回切换，导致无法充分利用cache
- SMP环境下，多个CPU操作同一队列需要加自旋锁，浪费CPU资源
- 不能及时地响应实时任务
- 调度不公平，更倾向于执行I/O密集型任务。

## O(1)算法分析
#### 优先级队列结构

```c
#define MAX_USER_RT_PRIO    100
#define MAX_RT_PRIO         MAX_USER_RT_PRIO
#define MAX_PRIO            (MAX_RT_PRIO + 40)
 
#define BITMAP_SIZE ((((MAX_PRIO+1+7)/8)+sizeof(long)-1)/sizeof(long))
 
struct prio_array {
    int nr_active;
    unsigned long bitmap[BITMAP_SIZE];
    struct list_head queue[MAX_PRIO];
};
 
typedef struct prio_array prio_array_t;
```

分为140个优先级（0~139，数值越小优先级越高），每个优先级都维护一个队列，其中利用了一个bitmap位图可以快速地以O(1)时间得到哪一个任务队列是非空的（比如可利用某些体系机构的专用指令）。

![image](https://img-blog.csdnimg.cn/20210321123714645.png)

#### runqueue结构

```c
struct runqueue {
    spinlock_t lock;
    unsigned long nr_running,
                  nr_switches,
                  expired_timestamp,
                  nr_uninterruptible;
    task_t *curr, *idle;
    struct mm_struct *prev_mm;
    prio_array_t *active, *expired, arrays[2];
    int prev_cpu_load[NR_CPUS];
    task_t *migration_thread;
    struct list_head migration_queue;
    atomic_t nr_iowait;
};
```

每一个CPU都会维护一个runqueue结构，包含优先级队列、调度次数、CPU负载等信息，其中比较重要的字段为`prio_array_t *active, *expired, arrays[2];`

![image](https://img-blog.csdnimg.cn/20210321123927239.png)

active为可以调度的任务队列，expired为时间片已经用完的任务队列。其规则如下：
- 当active中的任务时间片用完时，会被移动至expired中
- 当active中已经没有任务可以运行，就把expired和active指针交换

#### 完整的步骤
- 在 active bitarray 里，寻找 left-most bit 的位置 x。
- 在 active priority array（APA）中，找到对应队列 APA[x]。
- 从 APA[x] 中 dequeue 一个 process，dequeue 后，如果 APA[x] 的 queue 为空，那么将 active bitarray 里第 x bit置为 0。
- 对于当前执行完的 process，重新计算其 priority，然后 enqueue 到 expired priority array（EPA）相应的队里 EPA[priority]。
- 如果 priority 在 expired bitarray 里对应的 bit 为 0，将其置 1。
- 如果 active bitarray 全为零，将 active bitarray 和 expired bitarray 交换一下。

#### 实时进程调度
实时进程分为两种FIFO和RR，0~99位实时进程优先级
- FIFO实时进程的调度：一直运行该进程直到主动放弃（退出运行或者sleep()）
- RR进程：分配时间片，时间片用完时重新分配并放入active队列

#### 普通进程调度
100~139为普通进程优先级。动态优先级计算公式：`动态优先级 = max(100, min(静态优先级 – bonus + 5), 139))`，其中bonus由进程的睡眠时间计算得来（睡眠越久，优先级自然越高）

#### 时间片计算

```
静态优先级 < 120，运行时间片 = max((140-静态优先级)*20, MIN_TIMESLICE)
静态优先级 >= 120，运行时间片 = max((140-静态优先级)*5, MIN_TIMESLICE)
```

#### 时钟中断代码

```
//时钟中断会触发调用 scheduler_tick() 内核函数
void scheduler_tick(int user_ticks, int sys_ticks)
{
    runqueue_t *rq = this_rq();
    task_t *p = current;
 
    ...
 
    // 处理普通进程
    if (!--p->time_slice) {                // 减少时间片, 如果时间片用完
        dequeue_task(p, rq->active);       // 把进程从运行队列中删除
        set_tsk_need_resched(p);           // 设置要重新调度标志
        p->prio = effective_prio(p);       // 重新计算动态优先级
        p->time_slice = task_timeslice(p); // 重新计算时间片
        p->first_time_slice = 0;
 
        if (!rq->expired_timestamp)
            rq->expired_timestamp = jiffies;
 
        // 如果不是交互进程或者没有出来饥饿状态
        if (!TASK_INTERACTIVE(p) || EXPIRED_STARVING(rq)) {
            enqueue_task(p, rq->expired); // 移动到expired队列
        } else
            enqueue_task(p, rq->active);  // 重新放置到active队列
    }
    ...
}
```

#### 任务调度代码

```
void schedule(void)
{
    ...
    prev = current;  // 当前需要被调度的进程
    rq = this_rq();  // 获取当前CPU的runqueue
 
    array = rq->active; // active队列
 
    // 如果active队列中没有进程, 那么替换成expired队列
    if (unlikely(!array->nr_active)) {
        rq->active = rq->expired;
        rq->expired = array;
        array = rq->active;
        rq->expired_timestamp = 0;
    }
 
    idx = sched_find_first_bit(array->bitmap); // 找到最高优先级的任务队列
    queue = array->queue + idx;
    next = list_entry(queue->next, task_t, run_list); // 获取到下一个将要运行的进程
 
    ...
    prev->sleep_avg -= run_time; // 减少当前进程的睡眠时间
    ...
 
    if (likely(prev != next)) {
        ...
        prev = context_switch(rq, prev, next); // 切换到next进程进行运行
        ...
    }
    ...
}
```

#### O(1)进程调度算法总结
虽然算法极大地减小了时间复杂度，但是在历史上很快被CFS调度所取代，原因在于：
- 复杂的代码：主要来自于动态优先级的计算，根据平均睡眠时间和难以理解的经验公式来修正以区分交互式进程。
- 调度的不公平性
- 调度器性能在很多情况下会失效

#### 参考
- [https://blog.csdn.net/Rong_Toa/article/details/115047617](https://blog.csdn.net/Rong_Toa/article/details/115047617)
- [https://zhuanlan.zhihu.com/p/33461281](https://zhuanlan.zhihu.com/p/33461281)


##  两个完全公平算法的尝试
#### 楼梯调度算法SD
- 时间复杂度同样为O(1)，但极大简化了代码，甚至被称为O(1)算法的补丁，虽热历史上存在的时间很短，但是却具有重要意义：证明了完全公平算法的可行性。
- 维护若干个基于优先级的队列，存储在active列表中，每次取下一个进程时同样直接从active中直接读取。每次用完自己的时间片后，被加入低一级的优先级队列中，但自身的优先级不变。依次类推，当处于最低一级优先级队列中的进程的时间片用完时，则重新分配时间片加入比初始优先级低一级的队列中，如此循环。
- ==避免了进程饥饿==，因为高优先级进程最终会和低优先级进程竞争，直到低优先级进程得到执行。
- ==保证了交互式应用的高响应度==。交互式进程阻塞后，与它相同优先级的进程在一步步走下台阶，当它被唤醒时，大概率能够得到立即的调度执行。

#### 旋转楼梯最终时限调度算法RSDL
重新引入expire数组，每一个优先级都分配了组时间配额Tg，同一优先级的每一个进程具有同样的时间配额Tp.优势在于：在SD算法中，低优先级进程需要等待高优先级进程全部执行完毕才能得到调度执行，而这个等待时间无法估计，==但在RSDL中，高优先级组用完Tg后，将把其中所有的进程全部降低到下一个优先级队列，改善了调度的公平性==。

## CFS完全公平调度算法
见【Linux内核】CFS完全公平调度算法，该算法是最主要的调度器。

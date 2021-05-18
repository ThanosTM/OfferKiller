## 概念
> 在真实的硬件上实现理想的、精准、完全公平的多任务调度

#### 静态优先级与进程权重

```c
static const int prio_to_weight[40] = {
     /* -20 */     88761,     71755,     56483,     46273,     36291,
     /* -15 */     29154,     23254,     18705,     14949,     11916,
     /* -10 */      9548,      7620,      6100,      4904,      3906,
     /*  -5 */      3121,      2501,      1991,      1586,      1277,
     /*   0 */      1024,       820,       655,       526,       423,
     /*   5 */       335,       272,       215,       172,       137,
     /*  10 */       110,        87,        70,        56,        45,
     /*  15 */        36,        29,        23,        18,        15,
};
```

nice值越小，进程的优先级越高，进程的权重就越大。

#### 一个进程在一个调度周期中的运行时间计算
==分配给进程的运行时间 = 调度周期 * 进程权重 / 所有进程权重之和==，其中调度周期为固定值，因此进程的权重越大，分配得到的运行时间也越大。

#### 虚拟运行时间vruntime 
==vruntime = 实际运行时间 * NICE_0_LOAD / 进程权重==，因此进程权重越大，运行同样的实际时间，vruntime增长得更慢。CFS的思想就是让每个调度实体（没有组调度的情形下就是进程，以后就说进程了）的vruntime==互相追赶==，而每个调度实体的vruntime增加速度不同，权重越大的增加的越慢，这样就能获得更多的cpu执行时间

#### 完全公平的精髓

```
vruntime = 进程在一个调度周期内的实际运行时间 * 1024 / 进程权重
         = (调度周期 * 进程权重 / 所有进程总权重) * 1024 / 进程权重 
         = 调度周期 * 1024 / 所有进程总权重
```

因此，所有进程的vruntime值大小理想情况下都是一样的，即完全公平。

## 组织结构
#### 完全公平运行队列`struct cfs_rq`
运行在同一个cpu上的处于TASK_RUNNING状态的普通进程的各种运行信息
```c
struct cfs_rq {
    struct load_weight load;  //运行队列总的进程权重
    unsigned int nr_running, h_nr_running; //进程的个数

    u64 exec_clock;  //运行的时钟
    u64 min_vruntime; //该cpu运行队列的vruntime推进值, 一般是红黑树中最小的vruntime值

    struct rb_root tasks_timeline; //红黑树的根结点
    struct rb_node *rb_leftmost;  //指向vruntime值最小的结点
    //当前运行进程, 下一个将要调度的进程, 马上要抢占的进程, 
    struct sched_entity *curr, *next, *last, *skip;

    struct rq *rq; //系统中有普通进程的运行队列, 实时进程的运行队列, 这些队列都包含在rq运行队列中  
    ...
};
```

#### 调度实体`struct sched_entity`
记录一个进程的运行状态信息
```c
struct sched_entity {
    struct load_weight  load; //进程的权重
    struct rb_node      run_node; //运行队列中的红黑树结点
    struct list_head    group_node; //与组调度有关
    unsigned int        on_rq; //进程现在是否处于TASK_RUNNING状态

    u64         exec_start; //一个调度tick的开始时间
    u64         sum_exec_runtime; //进程从出生开始, 已经运行的实际时间
    u64         vruntime; //虚拟运行时间
    u64         prev_sum_exec_runtime; //本次调度之前, 进程已经运行的实际时间
    struct sched_entity *parent; //组调度中的父进程
    struct cfs_rq       *cfs_rq; //进程此时在哪个运行队列中
};
```

#### 红黑树组织
红黑树具有优秀的性质，保证了树的均匀分布，不会出现极端的性能问题。
![image](https://img-blog.csdn.net/20150726164218325)
调度器只需要每次选取最左边的节点进行调度执行即可，复杂度为O(logN)，事实上，该节点将被缓存，因此复杂度为O(1)；每次调整红黑树的时间复杂度为O(N).

## 过程
#### 创建新进程
创建新进程时, 需要设置新进程的vruntime值以及将新进程加入红黑树中. 并判断是否需要抢占当前进程

#### 进程唤醒
唤醒进程时, 需要调整睡眠进程的vruntime值, 并且将睡眠进程加入红黑树中. 并判断是否需要抢占当前进程

#### 进程的调度
进程调度时, 需要把当前进程加入红黑树中, 还要从红黑树中挑选出下一个要运行的进程.

#### 时钟周期中断
在时钟中断周期函数中, 需要更新当前运行进程的vruntime值, 并判断是否需要抢占当前进程


## 细节
#### 模块化思想
![image](http://mmbiz.qpic.cn/mmbiz_png/Ass1lsY6byuVTWDRZgQx3H63HrCvibHXICBuAgxZXT45m0Vs7WORkEPLD7A2pDcbhWlqGHIV9cjEC5vVzic5ke1g/640?wx_fmt=png&tp=webp&wxfrom=5&wx_lazy=1&wx_co=1)

分为两个层次，Core Scheduler是对所有Task共同逻辑的抽象，同时各种特定的类型又可以定义自己的sched_class，以链表的形式加入到系统中，而不需要改动Core的代码。

#### 关于公平
公平的精髓在于追求CPU资源分配的公平，引入load_weight的概念，保证所有可运行下的进程按照权重分配CPU资源。

#### 时间粒度
从一个粗时间粒度来看CFS调度保证了进程公平分配CPU的时间，CFS的时间粒度叫作调度周期。

#### 如何保证有界的调度延迟
- 传统的调度器是无法保证调度延迟的有界性的，例如固定时间片尾100ms，则当两个进程参与调度时，延迟为100ms，而当四个进程参与调度时，延迟为300ms；于是调度延迟与系统的负载有关。
- CFS确保有界的调度延迟：保证在一个target_lantency中，所有的runnable进程都会至少得到一次执行。该target_lantency的计算如下：
```c
static u64 __sched_period(unsigned long  nr_running)
{
    if  (unlikely(nr_running > sched_nr_latency))
        return  nr_running * sysctl_sched_min_granularity;
    else
        return  sysctl_sched_latency;
}
```
其中sysctl_sched_latency = 6ms，sysctl_sched_min_granularity = 0.75ms即一个进程被调度至少执行的时间片大小。

## 参考
- [https://blog.csdn.net/liuxiaowu19911121/article/details/47070111](https://blog.csdn.net/liuxiaowu19911121/article/details/47070111)
- [http://blog.chinaunix.net/uid-24708340-id-3787960.html](http://blog.chinaunix.net/uid-24708340-id-3787960.html)
- [https://www.jianshu.com/p/9114254410ff](https://www.jianshu.com/p/9114254410ff)

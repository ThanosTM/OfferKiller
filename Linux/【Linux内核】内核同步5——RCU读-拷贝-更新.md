## RCU的引入
#### 基于spinlock的性能问题
当线程在多个CPU上竞争spinlock进入临界区时，均会对这个lock进行操作，若CPU 0竞争到了lock并对其进行修改时，为了数据的一致性，CPU 0的操作会使其他CPU的cache行无效，必须从下一个Level的cache中取数据，而这将造成大量的开销；而RCU在读侧不需要访问这样的共享数据，从而极大地提升了读侧的性能。

#### reader和writer可以并发访问
rwlock允许多个读者进行并发访问，提升了内核的并发性，但RCU允许多个读者和一个写者的并发访问，进一步提升了性能，如图所示：

![image](http://www.wowotech.net/content/uploadfile/201512/8180fbe55bc857b2d0208565308c932420151203045708.gif)

#### 使用场景
RCU源自CPU和存储器件的速度差别逐渐增大，基于Counter的锁机制不能满足性能要求，从而提出的lock-free的同步方法，但使用场景也较为受限：
- RCU只保护动态分配的数据结构，并且由指针进行访问；
- 受RCU保护的临界区不能睡眠（但也有可睡眠的RCU叫SRCU）；
- 读写不对称，reader性能极高，但对writer没有性能要求；
- reader端对旧数据不敏感；

## 实现机制
![image](http://www.wowotech.net/content/uploadfile/201512/015011aa8857e3c6b7a37825ec01a6cf20151203045710.gif)

RCU的精髓在于，将写操作分为两个独立的步骤：removal和reclaimation.

读者几乎不做任何事情来防止竞争条件的出现，而写者不得不做更多的事情：当写者需要更新数据结构时，先要获得整个数据结构的完整副本，在对新副本进行修改，修改完毕以后通过切换指针的方式（这是一个原子操作）对其他CPU可见，此后的reader访问的就是新副本了；==最关键的问题在于：旧副本何时回收？== 因为可能还有reader在访问旧副本，需要等到所有旧读者（新读者已经在访问新副本了）从就副本中离开后，再释放旧副本的数据结构！这种等待方式有两种：
- synchronize_rcu()：写者显式等待旧读者全部离开，这个函数返回时说明所有的旧读者已经离开；
- call_rcu()：某些情况下writer无法阻塞，该函数仅仅注册完回调函数就离开，并在恰当的一个推后时机进行reclaimation，由tasklet实现！

#### reader基本RCU操作
- rcu_read_lock：用来标识RCU read side临界区的开始；
- rcu_dereference：该接口用来获取RCU protected pointer，对该指针进行解引用以访问临界区；
- rcu_read_unlock：用来标识reader离开RCU read side临界区；

#### writer基本RCU操作
- rcu_assign_pointer：完成数据分配与更新后，调用该接口暴露新副本指针；
- synchronize_rcu()；
- call_rcu()；

## RCU是Linux2.6的新功能，大量用于网络层和虚拟文件系统中

## 更底层一些吧
#### 两个重要的概念
- ==Grace Period，GP，宽恕期==：表示从写者更新到所有旧读者离开的时间段，GP结束后就将回收旧副本
- ==Quiescent State，QS，静止状态==：使用RCU时约定不能阻塞、不能切换到用户态、不能idle，进入静止状态被认为这个CPU已经退出了临界区，当所有CPU退出临界区后，可以认为GP结束，可以回收旧副本

#### CPU结点的树形管理
![image](https://pic4.zhimg.com/v2-c1baa45b0db8992aec074cef90b95ea9_r.jpg)

早期SMP系统的RCU实现是使用一个全局的bitmask来标记哪些CPU经历了QS，在方位这个全局bitmask时需要互斥访问，随着CPU数量的增多效率就会越来越低；因此引入树形管理，将所有CPU分而治之，同一组的CPU竞争bitmask，不同组的不需要；CPU由结点管理，叶节点通过上报的方式告知根节点，逐层传递。一个叶节点管理16个CPU。

#### CPU如何检测QS
执行修改CPU是否经过QS的函数有为cu_xxx_qs()，主要在这些时间点进行检测：
- 定时检查tick_sched_timer();
- 上下文切换时检查__schedule();
- 完成软中断时进行检查irq_exit();
- rcu_read_unlock()时检查;

#### QS如何上报
QS状态按照cpu->leaf->node->root->state的顺序逐级上报。

#### GP状态管理
每种flavor的RCU使用一个单独的内核线程来管理GP状态，具体形式为状态机的跳转（略）

#### 发布与订阅
所谓的订阅和发布在API上的体现是rcu_assign_pointer和rcu_dereference，从名字中就可以看出本质上就是指针赋值与指针解引用，那为什么要大费周折地包装起来呢？==防止在并发读写下，reader读到updater的半完成数据。== 实现的机制是内存屏障barrier，在ARM架构中的实现为：smp_mb()就包含了barrier
```c
#define smp_store_release(p, v)                     \
do {                                    \
    compiletime_assert_atomic_type(*p);             \
    smp_mb();                           \
    WRITE_ONCE(*p, v);                      \
} while (0)
```

指令重排这是基于两个原因的：
- 编译器优化：编译器可能并不认识updater实际上是在并行环境下运行的
- CPU乱序执行
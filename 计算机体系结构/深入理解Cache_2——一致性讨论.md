## Cache和DMA的一致性问题
DMA，Direct Memory Access，直接存储器访问，外设可以不用CPU干预，直接与内存进行数据传输，这个过程可以将CPU解放出来，进而提高系统的性能。

#### 一致性问题
![image](https://pic1.zhimg.com/v2-5c8283dee4ca2cc45bf2dc81c24d50a8_b.jpg)

- DMA读情形：若cache采用写回机制，修改过的数据还留在cache中，若cache与DMA没有任何关系，则DMA从内存获取到的是旧的数据；
- DMA写情形：DMA将数据写入内存，但cache中可能恰好存在原本该内存区域的有效行，导致下次CPU访问时读到的仍然是旧行。

#### 总线监视技术
总线监视技术即Cache控制器监视总线上的每一条内存访问（包含CPU和DMA），然后检查是否命中。由于DMA操作是物理地址，则cache控制器也应该是物理地址查找的，即PIPT Cache可以实现总线监视技术。当然不存在别名问题的VIPT也是可以的，Cache大小小于页面大小4KB。总线监视技术对于软件是透明的，软件不需要进行任何干涉即可避免一致性问题。但是并不是所有硬件都支持总线监视，操作系统应该在不支持总线监视的硬件上使用软件方法进行避免。

#### 最简单的方法：nocache
在使用DMA时，往往会先申请一段内存作DMA Buffer，为了避免cache影响，可以将这段内存设置为nocache。这种方法虽然简单，但是如果只是偶尔使用DMA，由于nocache会导致性能缺失，这是Linux中`dma_alloc_coherent()`的实现方法。

#### 软件维护cache一致性
仍然采用cache，但是会根据DMA传输方向的不同，采取不同措施进行维护：
- DMA负责I/O设备到内存的数据传输：在DMA传输之前invalid掉DMA Buffer区域内的高速缓存，在DMA传输完成后，程序再读取数据时就不会因为cache hit而访问到过时的数据。
- DMA负责内存到I/O设备的数据传输：在DMA传输之前，先显式clean掉DMA Buffer内存区域的高速缓存，clean的作用是显式回写，此时内存中的数据和Cache中已经一致了，再进行DMA传输操作。
- 这里需要注意的是在DMA传输期间CPU不要访问DMA Buffer，可以分别考虑一下两种传输方向时会出现什么问题。这是Linux系统中流式DMA映射`dma_map_single()`的实现方式。

#### DMA Buffer的对齐要求
![image](https://pic4.zhimg.com/v2-1f8448e46a2d731a9c60a6894cf4a973_b.png)

如上图所示，若变量temp和将要用于DMA操作的Buffer在同一Cacheline，上面描述过在DMA传输期间不能访问DMA Buffer，可是这种情况下访问这个temp事实上也会造成同样的问题。因此DMA Buffer需要以cacheline大小对齐。例如在Linux中，规定DMA Buffer不能从栈或者全局变量上分配，因为无法保证对齐性。我们需要通过`kmalloc()`分配，Linux提供了`ARCH_DMA_MINALIGN`宏定义用于规定分配object的最小size，在ARM64上这个值为128，因此ARM64平台分配小内存代价还是挺高的。而x86_64不用定义，因为其硬件确保了DMA的一致性。


## iCache和dCache的一致性问题
从多级Cache架构图中可知，L1 Cache比较特殊，它进一步分成了指令iCache和数据dCache，它由每个CPU所私有。

#### 区分iCache和dCache的原因
- 可以同时获取指令和数据，做到硬件上的并行，提升性能；
- 指令一般具有只读的特性，因此iCache在硬件上可以做成只读的。

#### iCache的歧义与别名问题
- 是否存在歧义问题？使用物理Tag避免了歧义问题。
- 是否存在别名问题？别名问题即一块物理地址映射多个虚拟地址导致一致性问题，但是iCache是只读的，因此就算有多个Cacheline加载了相同物理地址的指令，也是没有机会进行修改的，于是不存在别名问题。

#### iCache和dCache的一致性问题
大部分情况下是不存在问题的，因为指令一般不可修改；但在某些场景下，指令是self-modifying的，即代码在执行过程中会自我修改，例如GCC调试打断点的时候就需要修改指令。修改指令的步骤为：
- 将需要修改的指令加载到dCache；
- 修改成新指令，写回到dCache；

由此产生两个问题：
- 旧指令已经缓存在iCache中，程序执行依然命中iCache.
- iCache缓存不命中，则从主存中读取，但是若dCache采用回写且新指令还在dCache中，则读取到的仍然是旧指令。

#### 硬件维护一致性
让iCache和dCache相互通信，当修改dCache时查找iCache是否命中，如果命中则也更新iCache（解决了问题1）；当加载指令时iCache miss，则再到dCache中查找，再次miss则从主存中读取（解决了问题2）。虽然确实解决了冲突问题，而且不需要软件的维护，但是self-modifying的场景是极少数，却增加了硬件的成本与负担，不划算，一般均采用软件维护方法。

#### 软件维护一致性
OS发现修改的数据可能是代码时，采取下面步骤维护一致性：
- 将需要修改的指令数据加载到dCache中
- 修改成新指令，写回dCache。
- clean dCache中修改的指令对应的cacheline，保证dCache中新指令写回主存。
- invalid iCache中修改的指令对应的cacheline，保证从主存中读取新指令。

最后一个问题：OS如何知道修改的数据可能是代码？==代码段所在的页具有可执行权限==，而其他也一般只具有读写权限，由此可以得知修改的可能是指令，进而采取上述措施维护一致性。


## 多核Cache一致性问题
进入后摩尔时代，人们试图增加CPU核心数来提升系统的整体性能，此为多核系统。对于多核系统而言，每个CPU都有一个私有的Cache，这就产生了多个Cache中数据的一致性问题。

#### 问题描述
- 假设两个CPU都要读取0x40处的数据，导致0x40开始的一断内存分别加载到两个CPU各自私有的Cache中。随后CPU0进行写操作，CPU0更新其Cache中的值，但CPU1此时如果读取该值将会发生cache hit，进而读取到旧值，这是便发生了两个CPU cache中数据不一致的问题。
- 软件维护一致性VS硬件维护一致性：软件维护的成本太高，带来的性能损失几乎抵消掉了使用cache所带来的，因此只考虑使用硬件维护多核Cache一致性问题。

#### 广而告知：Bus Snooping
![image](https://pic3.zhimg.com/80/v2-562854145b89495da3bb2792da537466_720w.jpg)

在上个例子中使用Bus Snooping机制后，当CPU0修改其私有的Cache时，硬件就会通过总线广播通知其他所有CPU，对于每个CPU其硬件监听广播事件，若CPU1嗅探发现自己Cache中缓存了相同的数据，则将更新对应的cacheline。该协议虽然简单，但是需要每时每刻监听总线上的一切活动，且任何修改操作都需要进行一次广播，这增加了总线负担，增加了读写延迟，于是对其稍加改进可以引出MESI协议。

#### 精准投放：Directionary-based
![image](https://pic3.zhimg.com/80/v2-580c4c41e1f6abafdc9edcc71580b8aa_720w.jpg)

每个总线transaction只会发给感兴趣的CPU，每条cacheline存放了该缓存行的所有者Owner，和所有CPU的bitmap标识被多少个CPU所共享。最坏情况下，某块内存数据被所有CPU共享，所需要的总线带宽为N^2，相较snoop模式毫无优势。好在大多数时候，共享数据远不可能这么普遍，因此directionary-based比snoop节省总线带宽，但要付出查阅directionary的开销。对于NUMA系统一般采用基于directionary-based模式来维护一致性。

#### MESI协议
###### 状态字
MESI本质上是一个状态机：
- M：Modified，该Cacheline有效，数据被修改了，和内存中的数据不一致，数据只存在于本Cache中；
- E：Exclusive，该Cacheline有效，数据和内存中一致，数据只存在于本Cache中；
- S：Shared，该Cacheline有效，数据和内存中一致，数据被多个Cache共享；
- I：Invalid，该Cacheline无效；

==注意：== 对于M和E状态而言总是精确的，他们在和该缓存行的真正状态是一致的，而S状态可能是非一致的。如果一个缓存将处于S状态的缓存行作废了，而另一个缓存实际上可能已经独享了该缓存行，但是该缓存却不会将该缓存行升迁为E状态，这是因为其它缓存不会广播他们作废掉该缓存行的通知，同样由于缓存并没有保存该缓存行的copy的数量，因此（即使有这种通知）也没有办法确定自己是否已经独享了该缓存行。

从上面的意义看来E状态是一种投机性的优化：如果一个CPU想修改一个处于S状态的缓存行，总线事务需要将所有该缓存行的copy变成invalid状态，而修改E状态的缓存不需要使用总线事务。

###### MESI协议Messages
Cache之间通过发送Messages进行数据和状态同步沟通：
- Read：CPU要读取某个地址的数据
- Read Response：答复Read，返回所需数据
- Invalidate：请求其他CPU invalid某个地址的cacheline
- Invalidate Acknowledge：答复Invalidate，表明cacheline已经被Invalid
- Read Invalidate：Read+Invalidate
- Writeback：包含需要回写的地址和数据

###### 实例：
- 单核读取：CPU A发出Read，主存返回Read Response

![image](https://images2018.cnblogs.com/blog/1195582/201805/1195582-20180503162602668-681441242.png)

- 双核读取：CPU B发出Read，CPU0返回Read Response

![image](https://images2018.cnblogs.com/blog/1195582/201805/1195582-20180503162619534-683579600.png)

- 修改数据：CPU A发出Invalidate，CPU B将S改为I，回复Invalidate Acknowledge，CPU A收到后将S改为M

![image](https://images2018.cnblogs.com/blog/1195582/201805/1195582-20180503162633779-1465275811.png)

- 同步数据：

![image](https://images2018.cnblogs.com/blog/1195582/201805/1195582-20180503162644640-382839091.png)

当Cache状态为Modified和Exclusive时，修改其数据不需要发送消息给其他CPU，这在一定程度上减轻了带宽压力。

#### 了解即可：MESI协议的演化版
- AMD的Opteron处理器使用从MESI中演化出的MOESI协议，O(Owned)是MESI中S和M的一个合体，表示本Cache line被修改，和内存中的数据不一致，不过其它的核可以有这份数据的拷贝，状态为S。

- Intel的core i7处理器使用从MESI中演化出的MESIF协议，F(Forward)从Share中演化而来，一个Cache line如果是Forward状态，它可以把数据直接传给其它内核的Cache，而Share则不能。


## 基于MESI的atomic原子操作原理
详见【Linux内核】内核同步1——原子操作Atomic，这里主要讨论基于MESI的实现方式。

#### Bus Lock
当CPU发出一个原子操作时，先锁住总线，这样就可以防止其他CPU的内存操作，原子操作结束后释放总线。而实际上这种方法开销太大，我们只关心所操作的地址，其他无关的地址仍然能够进行数据读写，因此引入Cacheline lock.

#### Cacheline Lock
为了实现多核一致性，现在的硬件基本都采用MESI及其变种，我们可以借助Cache一致性协议MESI来实现原子操作。假设两个CPU，当CPU 0试图执行原子操作时，CPU 0发出Read Invalidate消息，其他CPU将原子变量所在的缓存行无效，并从Cache中返回数据，CPU 0将该cacheline置为Exclusive，同时将其标记Locked，直到原子操作结束后将其Unlock。期间若CPU 1也尝试进行原子操作，CPU 1发送Read Invalidate操作，CPU 0收到消息后检查对应的cacheline发现是锁住的，于是在解锁之前暂时不回复Invalidate Acknowledge。

#### LL/SC（Load-link/Store-Conditional）
这是另一种硬件支持原子操作的方法，ARM便采用该方式，详见【Linux内核】内核同步1——原子操作Atomic.


## 伪共享问题False Sharing
![image](https://pic2.zhimg.com/v2-5c0be1797fee007053920415c9beced7_b.jpg)

由上述讨论可知，多核Cache一致性由MESI协议确保，则考虑以下情形：内核地址空间是所有进程共享的，且由于设置了nG标志（见cache_1），这是一个Global映射，所有进程都能够cache hit。假设有两个变量gA和gB，他们在内存上是紧挨在一起的，且gA按cacheline size对齐，则gA和gB一定是被加载到同一缓存行的，假设有两个进程分别在两个CPU上运行，一个只访问gA，一个只访问gB，会发生什么问题呢？——==Cache颠簸==
- CPU 0访问gA，gA和gB被加载到CPU 0的cache中，这时该cacheline是Exclusive；
- CPU 1访问gB，gA和gB被加载到CPU1的cache中，并且CPU 0和CPU 1都将该cacheline标记为Shared；
- CPU 0修改gA，CPU 1的缓存行无效，CPU 0的cacheline标记为Modified；
- CPU 1修改gB，CPU 1该行无效，并且使CPU 0该cacheline同步到主存，再从主存读取该行（这其实取决于具体硬件实现，也可直接从CPU 0同步该行），此时CPU 0的标为Invalid，CPU 1的标为Modified.

如此往复，可以发现这两个全局变量的缓存行不断地在两个CPU之间移动，开销巨大，这就是典型的Cache颠簸问题（上一次发生该问题的场景是直接映射Cache），而实际上gA和gB完全没有任何关系，这是可以避免的。

#### 解决伪共享
抓住形成该现象的本质即可解决该问题：只需要让gA和gB不落在同一个cacheline即可，于是可以将gA和gB都按cacheline对齐，相当于以浪费一部分内存为代价，提升了系统的性能。

#### Linux系统的实现
在Linux kernel中存在__cacheline_aligned_in_smp宏定义用于解决false sharing问题。
```c
#ifdef CONFIG_SMP
#define __cacheline_aligned_in_smp __cacheline_aligned
#else
#define __cacheline_aligned_in_smp
#endif
```
可以看出，在单核中该宏定义为空，因为不需要考虑多核一致性问题。针对静态定义的全局变量，若在多核之间竞争比较严重，可以使用上述宏定义使变量按cacheline对齐。
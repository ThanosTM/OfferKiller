## MESI的后续讨论
继续考虑当时的场景：两个CPU同时读取某内存数据，此时该内存块被同时加载到各自Cache的cacheline中，并标记为Shared；此时CPU 0发出修改请求，CPU 1的缓存行被无效，而CPU 0的缓存行标记为Modified；最后让CPU 1读取该变量，则发生Cache miss，需要通知CPU 0做Write Back，再从主存同步该缓存行，最后两者都标记为Shared（或者将CPU 0标记为Invalid，从CPU 0得到数据，CPU 1变为Modified，这取决于硬件设计）。

#### Store Buffer
- 背景：若将上述场景最后一步改为：CPU 1对该共享变量进行写操作，则该写操作非常耗时（需要先将CPU 0的缓存行无效，CPU 1自己切换到Exclusive为后续写做准备），在这个耗时的操作过程中，CPU 1只是停留在写的动作，直到传输完成。==而实际上，该等待是毫无意义的，因为传递过来的数据马上就被写操作给覆盖掉了。==

![image](http://www.wowotech.net/content/uploadfile/201411/a872a1863fec02585bb786a5c382d3eb20141114112005.gif)

- 修改：写操作不必等待cacheline被加载，而是写到Store Buffer中后CPU就解放了，而当cacheline被加载后由硬件将Store Buffer中的内容写入cacheline。

- 引入逻辑错误：【==具体不理解？？？==】总之，Store Buffer的存在会造成修改只存在于Store Buffer中，未写到cache和内存上，导致CPU读取到旧值。

- 解决：引入确保清空Store Buffer的Memory Barrier.


#### Invalidate Queue
- 背景：为什么要引入Store Buffer：为了加快cache miss状态下的写性能，以便让CPU写写入Store Buffer而无需等待Invalidate Ack。然而Store Buffer比较小，若出现很多cache miss，瞬间就可以将Store Buffer填满，此时没有空间写了就只能等待Invalidate Ack了。为何Invalidate Ack如此之慢？因为先要对Cacheline进行invalidate操作，确保已经是invalid后再发送Ack，如果Cache正忙于做其他事，这个Ack出的就很慢了。

![image](http://www.wowotech.net/content/uploadfile/201411/b4c569d306427421b5b657fdcfce3cf120141114112010.gif)

- 修改：其他CPU发送到本CPU的invalidate命令可以先被推入Invalidate Queue，这个时候就不需要等待实际对cacheline的无效操作完成，CPU就可以回复Invalidate Ack了。

- 引入逻辑错误：【==具体不理解x2 ？？？==】总之，invalidate Queue的存在会造成invalidate停留在队列中，使得CPU无法读取到最新值。

- 解决：引入确保清空Invalidate Queue的Memory Barrier.


#### 总结
- Store Buffer位于CPU和Cache之间，对于x86架构来说，Store Buffer就是FIFO，即顺序写入；但是对于ARM/Power架构而言，Store Buffer并未保证FIFO写入，先写入Store Buffer的数据可能比后写入的晚刷入cache也是有可能的，这会让ARM/Power架构出现乱序。store barrier的意义就在于等待store buffer中的数据刷入cache.
- 某些CPU中Invalidate Queue的引入使得使cacheline失效的消息被缓存在队列中，还未应用到cacheline上，这也是一种可能出现乱序的原因。load barrier的意义就在于等待invalidate queue的命令应用到cacheline.
- 对于x86架构的CPU来说，单核上看保证了Sequential consistency，开发者完全不用担心单核上的乱序会造成正确性的问题；多核上看保证了x86-TSO模型，使用mfence就可以将Store Buffer中的数据刷入cache。且Store Buffer是FIFO的，也不存在Invalidate Queue，mfence就能够保证多核间数据的可见性与顺序性。
- 对于ARM和Power架构而言，编程变得危险许多，不仅单核上除了存在依赖的前后指令不能被乱序外，其余都有可能乱序，并且Store Buffer不是FIFO的，还可能存在Invalidate Queue，需要引入不同的barrier来完成不同的需求。


## 【TODO：内存一致性模型】==挖坑==
https://www.zhihu.com/column/cpu-cache
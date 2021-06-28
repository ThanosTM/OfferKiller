## 解决两个问题
- 单处理器下的乱序
- 多处理器下的内存同步

#### 乱序问题
- 编译器乱序：这是编译优化的手段，编译器试图通过指令重排将这样的两条指令拉开距离，以至于后一条指令进入CPU的时候，前一条指令的结果已经得到了，那么就不需要再阻塞等待了；
- 处理器乱序：现代处理器采用流水线技术与指令乱序执行的技术，虽然CPU顺序取指，但硬件分析之间的依赖关系后能够打乱指令顺序，将排在费时操作指令后面的指令提前执行。具体另开笔记学习吧。现在需要了解的是：就算编译器给出正确顺序的指令流，处理器也可能以乱序的方式进行执行。同时处理器在重排指令时会遵守程序的显示因果关系，以维持程序原有的逻辑；而无关的内存才会被乱序执行，但在CPU与IO设备交互时，这可能会成为问题，需要内存屏障对其进行干预以保证正确的执行顺序。

#### 多处理器内存同步问题
除了单处理器上指令有可能被乱序执行外，多处理器之间存在交互时也需要考虑乱序问题。这个问题来源于Cache一致性的机制（另开笔记写），简单来说CPU0对于变量A和B的更新并不是直接在内存上生效的，而是先要经过自身的cache，而CPU1对于A和B的访问也先要等A、B同步到内存，再同步到CPU1的cache，这其中的顺序是确定的。


## Linux内存屏障
- barrier()：优化屏障，阻止编译器为了进行性能优化而进行的指令重排；
- 读屏障：屏障之前的读操作一定会先于屏障之后的读操作；
- 写屏障：屏障之前的写操作一定会先于屏障之后的操作；
- 通用屏障

#### 应用一：网卡驱动中发送数据包
```c
/*
 * Writing to TxStatus triggers a DMA transfer of the data
 * copied to tp->tx_buf[entry] above. Use a memory barrier
 * to make sure that the device sees the updated data.
 */
wmb();
RTL_W32_F (TxStatus0 + (entry * sizeof (u32)),
       tp->tx_flag | max(len, (unsigned int)ETH_ZLEN));
```
wmb()保证在DMA传输之前，数据被完全写入buffer。

#### 应用二：睡眠进程的唤醒
```c
#define set_current_state(state_value)            \
    set_mb(current->state, (state_value))

#define set_mb(var, value)    do { var = value; smp_mb(); } while (0)
```
smp_mb()确保在current->state的值已经得到更新；


## ARM架构的内存屏障
![image](https://img-blog.csdnimg.cn/20200605170432374.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3UwMTIyNDc0MTg=,size_16,color_FFFFFF,t_70)

Linux内存屏障是依赖于体系结构的指令封装。




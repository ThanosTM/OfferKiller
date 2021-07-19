![image](https://static001.geekbang.org/resource/image/3c/0e/3c473d163b6e90985d7301f115ab660e.jpeg)

## 块设备的处理
#### 架构
![image](https://pic4.zhimg.com/80/v2-8bb7448134122e229368bc7797693a63_720w.jpg)

块设备的应用在Linux中是一个完整的子系统，从上到下依次是：
- 虚拟文件系统VFS：VFS位于块设备处理体系的上层，提供一个通用的文件模型；
- 磁盘高速缓存：有时候没有必要访问磁盘上的数据，因为内核将大多数最近从块设备读出或写入其中的数据保存在RAM中；
- 映射层mapping layer：目的是确定数据的物理位置，文件被看作拆分成的许多块组成，内核确定该文件所在文件系统的块大小与请求数据长度，随后调用具体的文件系统函数访问文件的磁盘结点，根据逻辑块好确定所请求的数据在磁盘上的位置。
- 通用块层generic block layer：内核利用此启动I/O操作来传送所请求的数据，每次I/O操作由一个bio来描述，bio负责收集底层组件所需要的所有信息以满足发出的请求。通用块层提供了一个隐藏硬件块设备之间差异的抽象视图。
- I/O调度程序层：试图将物理介质上相邻的数据请求聚集在一起。
- 块设备驱动程序：向磁盘控制器硬件发送适当的命令完成实际的数据传送。

#### 单位
![image](https://pic3.zhimg.com/80/v2-a12b50f4ff7cf254a8bf7c461b7a9a46_720w.jpg)

- 扇区Sector：硬件块设备控制器数据传送的基本单元，典型的大小为512字节。
- ==块Block==：VFS和文件系统传送数据的基本单位，对应着磁盘上一个或者多个相邻的扇区；Linux中块大小必须是2的幂次方，且不超过一个页框大小4KB，同时必须是扇区的整数倍大小。每个块有自己的==块缓冲区==，读取（硬件设备=>块缓冲区）与写入（块缓冲区=>硬件设备），==缓冲区首部==是一个与每个缓冲区相关的描述符；
- 段Segment：段是内存页中包含一些相邻磁盘扇区中的数据，这里有两种DMA传输方式，老式磁盘控制器只支持简单DMA传输，磁盘必须与RAM中连续内存单元相互传送数据；另外一种是“分散-聚集”DMA，一次能够同时传输几个段。


## 通用块层
#### `struct bio`
```c
/*
 * main unit of I/O for the block layer and lower layers (ie drivers and
 * stacking drivers)
 */
struct bio {
	sector_t		bi_sector;	/* device address in 512 byte
						   sectors */
	struct bio		*bi_next;	/* request queue link */
	struct block_device	*bi_bdev;
	unsigned long		bi_flags;	/* status, command, etc */
	unsigned long		bi_rw;		/* bottom bits READ/WRITE,
						 * top bits priority
						 */

	unsigned short		bi_vcnt;	/* how many bio_vec's */
	unsigned short		bi_idx;		/* current index into bvl_vec */

	/* Number of segments in this BIO after
	 * physical address coalescing is performed.
	 */
	unsigned int		bi_phys_segments;

	unsigned int		bi_size;	/* residual I/O count */

	/*
	 * To keep track of the max segment size, we account for the
	 * sizes of the first and last mergeable segments in this bio.
	 */
	unsigned int		bi_seg_front_size;
	unsigned int		bi_seg_back_size;

	unsigned int		bi_max_vecs;	/* max bvl_vecs we can hold */

	atomic_t		bi_cnt;		/* pin count */
	struct bio_vec		*bi_io_vec;	/* the actual vec list */
	bio_end_io_t		*bi_end_io;
};
```

bio是通用块层的核心数据结构，描述了块设备的I/O操作，其中较为重要的字段为：
- bi_io_vec：段数组
```c
/*
 * was unsigned short, but we might as well be ready for > 64kB I/O pages
 */
struct bio_vec {
	struct page	*bv_page;
	unsigned int	bv_len;
	unsigned int	bv_offset;
};
```
- bi_idx：指向待传送的第一个段

#### 磁盘与磁盘分区表示
通用块层处理的逻辑块设备称为磁盘，一个磁盘可以对一个硬件块设备，也可以是一个虚拟设备，建立在多个物理磁盘分区之上，在通用块层，磁盘这个概念完成了以上两种类型的统一。磁盘由`struct gendisk`描述：
```c
struct gendisk {
	/* major, first_minor and minors are input parameters only,
	 * don't use directly.  Use disk_devt() and disk_max_parts().
	 */
	int major;			/* major number of driver */
	int first_minor;
	int minors;                     /* maximum number of minors, =1 for
                                         * disks that can't be partitioned. */

	char disk_name[DISK_NAME_LEN];	/* name of major driver */
	char *(*devnode)(struct gendisk *gd, mode_t *mode);
	/* Array of pointers to partitions indexed by partno.
	 * Protected with matching bdev lock but stat and other
	 * non-critical accesses use RCU.  Always access through
	 * helpers.
	 */
	struct disk_part_tbl *part_tbl;
	struct hd_struct part0;

	const struct block_device_operations *fops;
	struct request_queue *queue;
	void *private_data;
```
- flags字段：描述了磁盘的信息，其中比较重要的是`GENHD_FL_UP`表示磁盘已被初始化并使用；
- part字段：指向磁盘的分区表，`hd_struct`描述了分区信息；
```c
struct hd_struct {
	sector_t start_sect;
	sector_t nr_sects;
	sector_t alignment_offset;
	unsigned int discard_alignment;
	struct device __dev;
	struct kobject *holder_dir;
	int policy, partno;
```

#### 提交请求
假设被请求的数据块在磁盘上是相邻的，并且我们已经知道了物理位置。
- `bio_alloc()`分配一个新的bio描述符，并初始化：起始扇区号、扇区数目、块设备描述符地址、段数组、==bi_rw设为请求操作标志==读或者写；
- `generic_make_request()`：这是通用块层的主要入口点
1. 检查扇区号有效性；
2. 获取请求队列q
3. `blk_partition_remap()`：完成磁盘分区的处理，之后的代码将忘记分区这回事，核心调整bio->bi_sector把相对于分区起始扇区号转变为相对于整个磁盘。
4. 调用q的`make_request_fn()`方法将bio插入请求队列。


## I/O调度程序
为了提升磁盘性能，内核试图把多个扇区合并在一起作为整体处理，减少磁头的平均移动时间。

#### 请求队列
每个块设备驱动程序维护自己的请求队列，请求队列由`struct request_queue`描述，请求队列是一个双向链表，链表中元素的排序方式对每个块设备驱动程序是特定的，但可以使用预先确定好的元素排序方式——I/O调度算法。

#### 请求
每个块设备的待处理请求均由`struct request`来描述：
- bio字段：每个请求包含一个或者多个bio，其实最初只有一个bio，当有新的请求时要么向bio中添加新段，要么再扩展一个新的bio；
- nr_sectors存放整个请求还需传送的扇区数，current_nr_sectors存放当前bio结构中还需传送的扇区数。
- flags是标志位，其中REQ_RW确定数据传送的方向。

#### 激活块设备驱动程序
延迟激活块设备驱动程序有利于把相邻块的请求进行集中，这种延迟依赖于设备的激活与撤销技术。`blk_plug_device()`函数用于插入一个块设备到某个块设备驱动程序处理的请求队列中，但此时该驱动程序并未被激活，而是启动一个内嵌定时器（典型3ms），直到超时再唤醒内核线程Kblockd操作的工作队列。

#### I/O调度算法
I/O调度程序每收到一条新的请求，试图通过扇区将请求队列排序，以提升磁盘的性能。I/O调度算法也叫电梯算法，Linux提供了4种不同的算法：
- Noop算法：最简单，不排序，直接取队列第一个请求就行；
- CFQ算法：完全公平队列，定义多个排序队列（例如64个），将不同进程通过Pid映射到某个队列，该进程发出的请求只被插入到相同的队列；每次轮询I/O输入队列以填充调度队列。
- “最后期限”算法：采用四个队列：两个读写排序队列，两个读写最后期限队列，当需要填充调度队列时进行如下步骤，确定请求的数据方向（优先读）、检查对应的最后期限队列，若已用完则添加该请求，若未用完则添加排序队列的下一个元素；
- 预期算法：Linux默认的电梯算法，最复杂；

#### 向I/O调度程序发出请求
前面==提交请求==的最后一步`make_request_fn()`方法向I/O调度程序发送一个请求，该方法的核心为`__make_request()`，若请求队列不空，则说明请求队列已经被拔掉或者很快将被拔掉（==？？？==），若请求队列不空，该函数插入请求队列；任何情形下，块设备驱动程序的策略例程最后都将处理调度队列中的请求。


## 块设备驱动程序
该层是Linux块设备子系统的最底层，从I/O调度程序中获得请求，然后按要求处理这些请求。

#### 块设备
![image](https://img-blog.csdnimg.cn/20181223170754450.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3UwMTIzMTk0OTM=,size_16,color_FFFFFF,t_70)

一个块设备需要处理多个块设备（磁盘的每个分区可以被看作是一个逻辑块设备），每个块设备由`struct block_device`描述：
```c
struct block_device {
	dev_t			bd_dev;  /* not a kdev_t - it's a search key */
	struct inode *		bd_inode;	/* will die */
	struct super_block *	bd_super;
	int			bd_openers;
	struct mutex		bd_mutex;	/* open/close mutex */
	struct list_head	bd_inodes;
	void *			bd_holder;
	int			bd_holders;
#ifdef CONFIG_SYSFS
	struct list_head	bd_holder_list;
#endif
	struct block_device *	bd_contains;
	unsigned		bd_block_size;
	struct hd_struct *	bd_part;
	/* number of times partitions within this device have been opened. */
	unsigned		bd_part_count;
	int			bd_invalidated;
	struct gendisk *	bd_disk;
	struct list_head	bd_list;
	/*
	 * Private data.  You must have bd_claim'ed the block_device
	 * to use this.  NOTE:  bd_claim allows an owner to claim
	 * the same device multiple times, the owner must take special
	 * care to not mess up bd_private for that case.
	 */
	unsigned long		bd_private;
};
```

- 若块设备对应一个磁盘分区，则`bd_contains`指向与整个磁盘相关的块设备描述符，`bd_part`指向`hd_struct`分区描述符；
- 若块设备对应一整个磁盘，则`bd_contains`指向自己，`bd_part`指向NULL；

#### 注册和初始化设备驱动程序
如何为一个块设备设计一个新的驱动程序：
- 自定义驱动程序描述符：包含驱动硬件设备所需的数据；
- 预定主设备号：这里不像字符设备一样，不能分配次设备号范围，主设备号和驱动程序也没有建立连接；
```c
    err = register_blkdev(FOO_MAJOR, "foo");
```

- 初始化自定义描述符：这里主要分配一个磁盘描述符，gendisk是块I/O子系统中最重要的数据结构，这其中也分配了`hd_struct`数组用于存放分区信息；
```c
    foo.gd = alloc_disk(16);
```

- 初始化gendisk描述符：主要设置一些主设备号、扇区大小等信息；
- 初始化块设备操作表：gd->fops；
- 分配和初始化请求队列：
```c
    foo.gd->rq = blk_init_queue(foo_strategy, &foo.lock);
```

- 设置中断处理程序；`request_irq()`；
- 注册磁盘：`add_disk(foo.gd)`，该函数主要是向内核注册gendisk描述符及其包含的对象中的kobject结构；

#### 策略例程
策略例程是块设备驱动程序的一组函数，与硬件块设备之间相互作用以满足调度队列中所汇集的要求，存放在`rd->request_fn`字段中。策略例程其实可以简单实现为对于调度队列中的每个元素进行数据传输，等待传输完成后继续处理下一个请求，直到调度队列为空；现代块设备驱动程序采用如下策略：
- 处理第一个请求，启动DMA传输，策略例程终止；
- 磁盘控制器中断，中断服务例程重新调用策略例程，要么请求再启动一次传输，要么从调度队列中删除该请求并开始处理下一个请求。

#### 中断处理程序
块设备的中断处理程序由DMA数据传输结束时被激活，检查是否已经传送完成所请求的所有数据块：是，则调用策略例程处理下一个请求；否则，更新当前请求的字段，并重新启动一次传输；
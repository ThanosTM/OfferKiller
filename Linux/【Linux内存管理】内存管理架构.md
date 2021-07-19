![image](https://pic4.zhimg.com/v2-38f29b01120f2f28f894837e468df618_1440w.jpg?source=172ae18b)

## UMA与NUMA
#### UMA
Uniform Memory Access，一致存储器访问：物理存储器被所有处理器共享，所有处理器对内存具有相同的访问时间。缺点在与在SMP体系中，可伸缩性是有限的，当存储器和I/O接口达到饱和时，增加处理器的数量并不能获得更高的性能。

#### NUMA
Nonuniform Memory Access，非一致存储器访问：分布式的存储器访问方式，每个CPU被划分成结点node，每个结点具有自己的本地存储器空间，具有较快的访问速度，所有结点通过互联网络连接，当需要访问远处结点的存储空间时，将通过互联网络导致附加的时延。

#### Linux内存管理架构
Linux内核通过插入一些兼容层，将不同体系结构间具体实现的差异隐藏起来，提供对一致和非一致内存访问的相同数据结构和接口。

- NUMA架构中，处理器被划分成node，分配有本地存储器空间；
- UMA架构中，相当于只是用了一个NUMA结点，完成了同一的内存管理

## Linux内存管理的组织形式
Linux把物理内存划分为3个层次进行管理
![image](https://pic2.zhimg.com/80/v2-5082cfdf3acf48c18739a0da3eddfedd_720w.jpg)

#### 存储结点node
CPU成多个结点，内存被分成簇，每个CPU对应一个本地物理内存，每个内存簇被认为是一个结点。

#### 管理区zone
每个物理内存节点node被划分为多个内存管理区域,用于表示不同范围的内存,内核可以使用不同的映射方式映射物理内存。

#### 页面page
内存被细分为多个页面帧,页面是最基本的页面分配的单位

## Node
#### 数据结构`pg_data_t`
Linux内核使用`pg_data_t`类型描述node，其中比较重要的字段有该结点的内存管理域、结点的内存页面、交换守护进程等等。
```c
typedef struct pglist_data {
	struct zone node_zones[MAX_NR_ZONES];
	struct zonelist node_zonelists[MAX_ZONELISTS];
	int nr_zones;
#ifdef CONFIG_FLAT_NODE_MEM_MAP	/* means !SPARSEMEM */
	struct page *node_mem_map;
#ifdef CONFIG_CGROUP_MEM_RES_CTLR
	struct page_cgroup *node_page_cgroup;
#endif
#endif
#ifndef CONFIG_NO_BOOTMEM
	struct bootmem_data *bdata;
#endif
#ifdef CONFIG_MEMORY_HOTPLUG
	/*
	 * Must be held any time you expect node_start_pfn, node_present_pages
	 * or node_spanned_pages stay constant.  Holding this will also
	 * guarantee that any pfn_valid() stays that way.
	 *
	 * Nests above zone->lock and zone->size_seqlock.
	 */
	spinlock_t node_size_lock;
#endif
	unsigned long node_start_pfn;
	unsigned long node_present_pages; /* total number of physical pages */
	unsigned long node_spanned_pages; /* total size of physical page
					     range, including holes */
	int node_id;
	wait_queue_head_t kswapd_wait;
	struct task_struct *kswapd;
	int kswapd_max_order;
} pg_data_t;
```

## Zone
为了解决实际计算机体系结构的诸多硬件限制，Linux内核对于不同区域的内存采用不同的管理方式和映射方式。具体地，使用了三种区：
- `ZONE_DMA`：用于执行DMA操作
- `ZONE_NORMAL`：正常能够用于映射的页
- `ZONE_HIGHMEM`：高端内存，不能永久映射到物理内存

对于x86_32体系，内存管理区分布如下：
![image](https://img-blog.csdn.net/20160831134445680)
![image](http://ilinuxkernel.com/wp-content/uploads/2011/09/091011_1614_Linux5.png)

- `ZONE_DMA`包含的页框可以由老式基于ISA的设备通过DMA使用；
- `ZONE_DMA`和`ZONE_NORMAL`包含内存的常规页框，将他们线性地映射到线性地址空间的第4个GB；
- 在具有大容量RAM的现代32位计算机中，CPU不能直接访问所有的物理内存，因为线性地址空间太小，内核空间只有1G，但却需要管理更大的物理内存，为此引入了高端内存的概念；在现代64位机中，线性地址空间暂时还远远超过系统的实际物理地址，因此`ZONE_HIGHMEM`总是空的。

#### 管理区结构`zone_t`
里面保存着内存使用状态信息，如page使用统计,未使用的内存区域，互斥访问的锁（LOCKS）等.
```c
struct zone {
     spinlock_t         lock;

     unsigned long      spanned_pages;
     unsigned long      present_pages; 
     unsigned long      nr_reserved_highatomic;    
     atomic_long_t      managed_pages;

     struct free_area   free_area[MAX_ORDER];
     unsigned long      _watermark[NR_WMARK];
     long               lowmem_reserve[MAX_NR_ZONES];
     atomic_long_t      vm_stat[NR_VM_ZONE_STAT_ITEMS];

     unsigned long      zone_start_pfn;
     struct pglist_data *zone_pgdat;
     struct page        *zone_mem_map;
     ...    
};
```
- `lock`：并行访问
- `spanned_pages`：这个zone含有的总得page frames数目
- `free_area`：由free list空闲链表组成，表示该zone还有多少空间可供分配的page_frames；
- `zone_start_pfn`是zone的起始物理页面号，zone_start_pfn+spanned_pages就是该zone的结束物理页面号；
- `zone_pgdat`是指向这个zone所属的node的；
- `zone_mem_map`指向由struct page构成的mem_map数组。

内核对于zone的访问是很频繁的，为了更好地利用硬件cache来提高访问速度，`struct zone`中还有一些填充位，帮助结构体元素与cache line对齐；这点与`struct page`对内存的精打细算的使用形成了鲜明的对比，因为zone的数量很有限，`struct zone`稍微大一点也没有关系。

## 高端内存
#### ==为什么需要高端内存==
Linux Doc中对highmem的描述为：
> High memory (highmem) is used when the size of physical memory approaches or exceeds the maximum size of virtual memory. At that point it becomes
impossible for the kernel to keep all of the available physical memory mapped
at all times. This means the kernel needs to start using temporary mappings of
the pieces of physical memory that it wants to access.

32位Linux将4GB的内存空间划分为内核空间和用户空间，其中内核空间为第4个GB：0xC0000000~0xFFFFFFFF，如果系统的物理内存大于1GB，就没有足够的内核线性地址来固定映射到全部物理内存和I/O空间了；x86平台中设置了一个经验值896MB，将前896MB的内核空间固定映射到物理内存，之后的内核空间不能够建立固定的映射，称为高端内存。

![image](https://img-blog.csdn.net/20160831143301132)

![image](https://img-blog.csdnimg.cn/20200514214159371.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3UwMTI0ODkyMzY=,size_16,color_FFFFFF,t_70)

#### 核心思想
==借一段地址空间，建立临时地址映射，用完后释放，达到这段地址空间可以循环使用，访问所有物理内存。==

#### 高端内存映射的3种方式
内核将高端内存划分为3个部分：
- VMALLOC_START~VMALLOC_END：被vmalloc()函数分配的物理上不连续，但线性空间连续的高端内存空间；
- KMAP_BASE~FIXADDR_START：被kmap()用于永久映射；
- FIXADDR_START~4G：被kmap_atomic()函数用来临时映射高端物理内存
- 其他未用高端线性地址空间可以用来在系统初始化期间永久映射I/O地址空间。

###### 永久内核映射
- 建立永久内核映射可能阻塞当前进程，因此不能用于中断处理程序和可延迟函数
- 永久内核映射允许内核建立高端页框到内核地址空间的长期映射；
- `kmap()`函数用于建立永久内核映射，内核专门预留了一块线性地址空间PKMAP_BASE~FIXADDR_START，该空间和其他空间使用同样的页目录表，这个空间为4M大小，通过kmap()最多可以同时映射1024个页表，因此对于不使用的页表应该及时释放。

###### 临时内核映射
- 不阻塞进程，可以在中断处理程序和可延迟函数中使用；
- 使用`kmap_atomic()`建立临时内核映射，内核确保同一窗口不会被两个不同的控制路径所使用。每一次映射会导致以前的映射被覆盖，因此这是一种临时的映射。


## Page
页框的信息保存在一个类型为page的页描述符中，所有页描述符存放在`mem_map`全局数组中。
```C
struct page {
    unsigned long flags;
    atomic_t count;  
    atomic_t _mapcount; 
    struct list_head lru;
    struct address_space *mapping;
    unsigned long index;         
    ...  
} 
```
虽然内存访问的最小单位为字节，但是MMU以page为单位来查找页表，page是Linux内存管理中的重要单位。因此该结构体非常重要，使用频率也极高。

- count：页引用计数器，-1表示页空闲，大于或等于0表示分配给了一个或者多个进程。
- flags：描述页框状态的标志位，例如锁定、刚刚访问、被修改（脏的）等等。包含PG_active, PG_dirty, PG_writeback, PG_reserved, PG_locked, PG_highmem等
- _mapcount：表示该页框被映射的个数；
- lru：least recently used，根据该页框的使用频率，一个可回收的页框要么挂在active_list上，要么挂在inactive_list上，这是页面回收的选择依据；

最新版本的Linux中该结构体中大量使用了union，也就是同一个元素在不同场景下具有不同的含义，这是因为每一个页框都需要用其来表示，在4GB物理内存的系统中，仅该项就要占据30MB以上，因此对这个结构体的设计必须非常考究，能复用的尽量复用。

#### ==必须要理解的是==
page结构与物理页相关，而并非与虚拟页相关，内核仅仅用这个数据结构来描述当前时刻在相关的物理页中存放的东西，目的在于描述物理内存本身；例如内核需要知道物理内存中所有的页是否空闲，若已被分配其所有者是谁，等等。

#### 保留的页框池
- 在多数情况下，请求页框时如果有足够的空闲内存可用则立即满足，否则将回收一些内存，并阻塞内核控制路径直到内存被释放；而当处理中断或执行临界区代码时，内核控制路径不能被阻塞，应该产生原子内存分配请求，原子请求不会被阻塞，也只会分配失败仅仅而已。
- 内核设法尽量减少这种原子内存请求不能满足的事件发生，因此保留了一个页框池，仅在内存不足时被使用。

#### 分区页框分配器
管理区分配器接收动态分配和释放的请求，该部分允许搜搜一个能满足一组连续页框内存的管理区，页框被“==伙伴系统==”来处理；为了达到更好的性能，一小部分页框保留在高速缓存中用于快速地满足对单个页框的分配请求（==每CPU页框高速缓存==）。常用的函数有：
`alloc_pages(gfp_mask, order)`、`alloc_page(gfp_mask)`等等。

## 伙伴系统
见【Linux内存管理】伙伴系统

## 每CPU页框高速缓存
所有每CPU高速缓存包含一些预先分配的页框，用于满足本地CPU发出的单一内存请求。使用每CPU的原因在于首先减少了数据锁定，只需要在调用get_cpu()时禁止内核抢占，就能够保证该cpu对属于自己变量的独占访问；其次是大大减少了缓存失效，percpu接口按cacheline对齐其数据，消除了cache伪共享现象。

#### 冷热高速缓存
每个内存管理区和每个CPU提供了两个高速缓存：
- 热高速缓存：内核或者用户态==刚分配到页框后就开始写==，该页框所包含的内容很有可能还在CPU硬件cache中
- 冷高速缓存：如果该页框将被==用作DMA填充==，则使用冷高速缓存是更好的，因为不涉及CPU因此不会弄脏硬件cache。

内核使用两个位标监视冷热高速缓存的大小，当页框低于下界low，则将通过伙伴系统分配batch个页框进行补充，若高于high则将释放batch个页面到伙伴系统中。

## 获取物理内存
#### __get_free_pages()
Linux为获取page frame提供了两个基本函数：
```c
struct page * alloc_pages(gfp_t gfp_mask, unsigned int order);
unsigned long __get_free_pages(gfp_t gfp_mask, unsigned int order);
```
这两个参数是一模一样的，只是在返回值上有所不同，`alloc_pages()`返回指向第一个page的`struct page`的指针，而`__get_free_pages()`返回的是第一个page映射后的虚拟地址，即多了一个地址转换工作，由于CPU直接使用的是虚拟地址，因此这么做是更方便的。
```c
unsigned long __get_free_pages(gfp_t gfp_mask, unsigned int order)
{
page = alloc_pages(gfp_mask, order);          
if (page ！= NULL)
	return (unsigned long) page_address(page);
}
```

#### vmalloc()
而vmalloc()分配大块内存时得到的物理内存不连续，他通过分配连续的物理内存块，再修正页表，将内存映射到逻辑地址空间的连续区域。该函数可能睡眠，因此不能在中断上下文中调用。
```c
for (i = 0; i < area->nr_pages; i++) {
     struct page *page;
     if (node == NUMA_NO_NODE)
	 page = alloc_page(alloc_mask);
     else 
         page = alloc_pages_node(node, alloc_mask, order);
}
```

#### kmalloc()
kmalloc(size, flags)则是按照字节进行分配，如果申请的空间比较小，则使用slab分配器，否则将使用alloc_pages()直接使用buddy system。当使用slab分配器时，kmalloc()会根据size大小去寻找一块合适的slab cachep来进行分配（因为slab cachep是用于固定大小内存分配的）。

#### gfp_mask
- GFP_KERNEL：最常用的标志，可能会引起睡眠或者阻塞，只能用于可以重新安全调度的进程上下文中（即没有持有锁）；当内存不足时可以让调用者睡眠、交换、刷新一些页到磁盘上。
- GFP_ATOMIC：不能睡眠的内存分配，相对受到严格限制，当内存不足时分配成功的概率也比较小。常用于中断处理程序、软中断、tasklet中。
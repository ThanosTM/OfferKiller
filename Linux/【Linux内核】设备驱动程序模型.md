## I/O体系结构
CPU和I/O设备的通过I/O总线相连，这种连接包含了3个硬件上的组织层次：I/O端口、I/O接口、设备控制器。

#### I/O端口
每个连接到I/O总线上的设备都也可以将有自己的地址集，即I/O端口port，CPU可以通过汇编指令对I/O端口进行读写，也可以映射到物理地址空间，像操作内存一样操作I/O端口；现代的硬件设备更偏向于后者——映射的I/O。内核使用resource来记录分配给每个硬件设备的I/O端口，该resource结构体被组织成一颗树状，因为对于一个地址范围而言，存在子范围隶属于该地址范围之下。当前分配给I/O设备的所有I/O地址的树都可以从/proc/ioports文件中获得。

#### I/O接口
I/O interface，是一组用于I/O端口和设备控制器之间的硬件电路，起到翻译的作用，将I/O端口中的值转换为设备所需要的指令和数据；分为两种：
- 专用I/O接口：如键盘接口、图形接口、网络接口等；
- 通用I/O接口：可以用来连接多个不同的硬件设备，如串口、USB口等；

#### 设备控制器
对于复杂的设备需要设备控制器完成高级命令和设备电信号的互相转换和解释，最典型的就是磁盘控制器。


## 设备驱动程序模型
- Linux为系统中所有总线、设备以及设备驱动程序提供了一个统一的视图；
- 引入设备驱动程序模型的意义在于对越来越多的硬件进行统一的实现和维护一些特性如：电源管理（控制设备电源线上不同的电压级别）、即插即用（配置设备时透明的资源分配）、热插拔（系统运行时支持设备的插入和移走）、对象生命周期、sysfs等，目的在于简化了驱动程序的编写、设备的管理等，但是本身的设计和实现非常复杂。

#### 底层架构
###### ==kobject==
`kobject`是设备驱动程序模型中最为核心的一个数据结构，是对该模型下所有对象抽出来的共有部分，相当于面向对象编程思想中的总基类（这个思想很重要！将会贯彻始终）。设备驱动程序模型下的对象只需包含该kobject即可完成以下工作：
- 引用计数器
- 维持层次列表或组
- 为容器的属性提供一种用户态查看的视图

```c
struct kobject {
    const char        *name;                   //   对象的名字
    struct list_head    entry;                 //   用来指向平行关系中的下一个kobject结构体对象(可以理解为同一个目录下的多个文件或者文件夹给他们链接起来)
    struct kobject        *parent;             //   用来指向父类对象(也就是他的上一层文件夹所对应的对象)
    struct kset        *kset;                  //   用来指向父类对象的kset
    struct kobj_type    *ktype;                //   指向一个kobj_type对象
    struct sysfs_dirent    *sd;
    struct kref        kref;                   //   kobject的引用计数
    unsigned int state_initialized:1;          //   该对象是否被初始化了  的状态标志位
    unsigned int state_in_sysfs:1;
    unsigned int state_add_uevent_sent:1;
    unsigned int state_remove_uevent_sent:1;
    unsigned int uevent_suppress:1;
};
```

![image](https://img-blog.csdn.net/20181010141506767?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQwNzMyMzUw/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)

###### kobj_type：kobject的属性
每个kobject所绑定，提供相应功能。
```c
struct kobj_type {
	void (*release)(struct kobject *kobj);//释放kobject和其占用的函数
	const struct sysfs_ops *sysfs_ops;  //操作一个属性数组的方法
	struct attribute **default_attrs;  //属性数组的方法
	const struct kobj_ns_type_operations *(*child_ns_type)(struct kobject *kobj);
	const void *(*namespace)(struct kobject *kobj);
};
```

关注两个点：
- sys_fops：提供该对象在sysfs中的show和store的方法；

```c
struct sysfs_ops{
ssize t (*show) (struct kobject *, struct attribute *, char *);/*读属性操作函数*/
ssize t (*store) (struct kobject *,struct attribute *,const char *buf, size_t count);/*写属性操作函数*/
}
```

- attribute：提供在sysfs中以文件形式存在的属性，应用接口

```c
struct attribute {
	const char		*name;  //属性的名字
	struct module		*owner;//指向用于该属性的模块，已经不常使用
	mode_t			mode;  //属性读写权限
};
```

###### kset
kset的主要作用是做顶层kobject的容器类，将各个kobject组织出目录层次结构，sysfs中一个文件夹形式存在的对象必须使用Kset包含。

```c
struct kset {
    struct list_head list;                    //   用来链接该目录下的所有kobject对象
    spinlock_t list_lock;                    //   自旋锁
    struct kobject kobj;                    //   这个kobject就是本目录对应的对象结构体
    const struct kset_uevent_ops *uevent_ops;
};
```

###### 描述kobject与kset之间的关系
![image](https://images2015.cnblogs.com/blog/944893/201611/944893-20161106144027018-851542704.jpg)
- kobject与kobject之间
- kset与kobject之间
- kset与kset之间

#### 组织方式
###### 设备device
```c

struct device {
    struct klist_klist children;/*连接子设备的链表*/
    struct device *parent;/*指向父设备的指针*/
    struct kobject kobj;/*内嵌的kobject结构体*/
    char bus_id[BUS ID SIZE];/*连接到总线上的位置*/ 
    unsigned uevent suppress:1;/*是否支持热插拔事件*/
    const char init_name;/*设备的初始化名字*/
    struct device_type *type;/*设备相关的特殊处理函数*/
    struct bus_type *bus;/*指向连接的总线指针*/
    struct device_driver *driver;/*指向该设备的驱动程序*/
    void *driver data;/*指向驱动程序私有数据的指针*/
    struct dev_pm info power;/*电源管理信息*/ 
    dev t deyt;/*设备号*/
    struct class *class;/*指向设备所属类*/ 
    struct attribute_group **groups;/*设备的组属性*/ 
    void (*release) (struct device *dev);/*释放设备描述符的回调函数*/
};
```
`struct device`是硬件设备在内核驱动框架中的抽象：
- 按照层次组织，子设备离开父设备就无法进行工作。
- 每个设备驱动程序保持一个device链表，其中的每个对象都是可被管理的设备，对于任何总线也能够访问到总线下的所有设备。
- `device_register()`函数的功能是向设备驱动程序模型中插入一个新的device对象，并在/sys/devices目录下创建新的目录。
- 通常device对象可以被嵌入更大的描述符中，例如usb、pci设备。

###### 驱动程序device_driver
`struct device_driver`是驱动程序的抽象：
```c

struct device_driver {
	const char		*name;//设备驱动程序的名字
	struct bus_type		*bus;//指向驱动属于的总线
 
	struct module		*owner;//设备驱动自身模块
	const char		*mod_name;	/* used for built-in modules */
 
	bool suppress_bind_attrs;	/* disables bind/unbind via sysfs */
 
#if defined(CONFIG_OF)
	const struct of_device_id	*of_match_table;
#endif
 
	int (*probe) (struct device *dev);//探测设备的方法，并检测设备驱动可以控制哪些设备
	int (*remove) (struct device *dev);//移除设备调用的方法
	void (*shutdown) (struct device *dev);//关闭设备的方法
	int (*suspend) (struct device *dev, pm_message_t state);//设备处于低功耗的方法
	int (*resume) (struct device *dev);//恢复正常的方法
	const struct attribute_group **groups;//属性组
 
	const struct dev_pm_ops *pm;//电源管理
 
	struct driver_private *p;//设备驱动私有数据
};
```
其中，probe()用于硬件探测程序，被总线设备驱动程序调用；remove()应用于热插拔移走或驱动程序卸载；而shutdown()、suspend()、resume()方法与设备供电状态的改变相关。

###### 总线bus
```c
struct bus_type {
    const char		*name;  //总线类型名
    struct bus_attribute	*bus_attrs;  //总线属性和导出到sysfs中的方法
    struct device_attribute	*dev_attrs;  //设备属性和导出到sysfs中的方法
    struct driver_attribute	*drv_attrs;  //驱动程序属性和导出到sysfs中的方法
     
    //匹配函数，检验参数2中的驱动是否支持参数1中的设备
    int (*match)(struct device *dev, struct device_driver *drv);
    int (*uevent)(struct device *dev, struct kobj_uevent_env *env);
    int (*probe)(struct device *dev);  //探测设备
    int (*remove)(struct device *dev); //移除设备
    void (*shutdown)(struct device *dev); //关闭函数
     
    int (*suspend)(struct device *dev, pm_message_t state);//改变设备供电状态，使其节能
    int (*resume)(struct device *dev);  //恢复供电状态，使其正常工作
     
    const struct dev_pm_ops *pm;  //关于电源管理的操作符
     
    struct bus_type_private *p;  //总线的私有数据
};
```
其中关注`match()`方法，用于内核检查一个给定的设备是否可以由给定的驱动程序处理，实现通常很简单，只需要在所支持标识符的驱动程序表中搜索设备的描述符。

###### 类class
相关结构体有`struct class`和`struct class_device`，udev的使用离不开类；类的概念是一组人为的划分，作为多个设备的一种容器，目的就是对各种设备进行分类管理。


#### 设备文件
根据设备的基本特性，可以分为三种：
- 字符设备：不可以被随机访问/能被随机访问但所需时间非常依赖于数据在设备内的位置（磁带）
- 块设备：能以差不多固定时间传送随机访问的数据块，典型的有磁盘、DVD播放器等等。
- 网络设备：是特殊设备的驱动，负责接收和发送帧，并不存在与/dev下面，也不是用read()等系统调用操作它，因此是一种例外。

设备标识符由设备文件的类型和一对参数表示，类型只有字符或者块，参数包含主设备号和次设备号，具有主设备号的设备可以认为由同一个设备驱动程序处理，次设备号则是具有相同主设备号中的一个特定设备。

#### 设备驱动程序
###### 注册
注册一个设备驱动程序意味着分配一个新的`device_driver`，这里分配静态编译进内核或者动态挂载内核模块。

###### 初始化
初始化意味着分配宝贵的系统资源，通过引用计数值来进行分配和释放，open()方法增加引用值，release()方法减少引用值。

#### 【面试题】字符设备和块设备的区别
本质在于是否能够被随机访问，即：能够在访问设备时从一个位置跳到另一个位置。键盘是典型的字符设备，因为键盘驱动程序会按照和输入完全相同的顺序返回按键的数据流，打乱或读取其他字符都是没有意义的；硬盘则是典型的块设备，要求能够读取硬盘上任意位置的块并跳到其他位置进行访问。内核管理块设备要比管理字符设备细致得多，需要考虑的问题和完成的工作相比字符设备来说要复杂许多。这是因为字符设备仅仅需要控制一个位置——当前位置；而块设备访问的位置必须能够在介质的不同区间前后移动。所以事实上内核不必提供一个专门的子系统来管理字符设备，但是对块设备的管理却必须要有一个专门的提供服务的子系统。不仅仅是因为块设备的复杂性远远高于字符设备，更重要的原因是块设备对执行性能的要求很高；对硬盘每多一分利用都会对整个系统的性能带来提升，其效果要远远比键盘吞吐速度成倍的提高大得多。另外，我们将会看到，块设备的复杂性会为这种优化留下很大的施展空间。

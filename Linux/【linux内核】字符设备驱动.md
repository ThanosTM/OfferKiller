## 基础
字符设备是指只能一个字节一个字节读写的、不能随机读取设备中某一数据块的、需要顺序读写的面向流的设备，常见的字符设备有：鼠标、键盘、串口、控制台、LED等。处理字符设备驱动相对比较容易，因为通常不需要复杂的缓冲策略，也不涉及磁盘高速缓存等。其驱动程序中完成的主要工作是初始化、添加和删除 struct cdev 结构体，申请和释放设备号，以及填充 struct file_operations 结构体中断的操作函数，实现 struct file_operations 结构体中的read()、write()和ioctl()等函数是驱动设计的主体工作。

## cdev
#### 数据结构
字符设备驱动程序由一个cdev结构描述：
```c
<include/linux/cdev.h>  

struct cdev {   
　　struct kobject kobj;                  //内嵌的内核对象.  
　　struct module *owner;                 //该字符设备所在的内核模块（所有者）的对象指针，一般为THIS_MODULE主要用于模块计数  
　　const struct file_operations *ops;    //该结构描述了字符设备所能实现的操作集（打开、关闭、读/写、...），是极为关键的一个结构体
　　struct list_head list;                //用来将已经向内核注册的所有字符设备形成链表
　　dev_t dev;                            //字符设备的设备号，由主设备号和次设备号构成（如果是一次申请多个设备号，此设备号为第一个）
　　unsigned int count;                   //隶属于同一主设备号的次设备号的个数
　　...
};
```

#### 接口
- 动态申请内存
```c
struct cdev *cdev_alloc(void);　　
/* 返回值：
　　　　成功 cdev 对象首地址
　　　　失败：NULL */
```

- 初始化cdev
```c
void cdev_init(struct cdev *p, const struct file_operations *p);　　
/* 参数：
　　　　struct cdev *p - 被初始化的 cdev对象
　　　　const struct file_operations *fops - 字符设备操作方法集 */
```

- 注册cdev
```c
int cdev_add(struct cdev *p, dev_t dev, unsigned count);
/* 参数：
　　　　struct cdev *p - 被注册的cdev对象
　　　　dev_t dev - 设备的第一个设备号
　　　　unsigned - 这个设备连续的次设备号数量
   返回值：
　　　　成功：0
　　　　失败：负数（绝对值是错误码）*/
```

#### cdev的fops成员
```c
struct file_operations {
　　struct module *owner;　　
　　　　/* 模块拥有者，一般为 THIS——MODULE */
　　ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);　　
　　　　/* 从设备中读取数据，成功时返回读取的字节数，出错返回负值（绝对值是错误码） */
　　ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);　　　
　　　　/* 向设备发送数据，成功时该函数返回写入字节数。若为被实现，用户调层用write()时系统将返回 -EINVAL*/
　　int (*mmap) (struct file *, struct vm_area_struct *);　　
　　　　/* 将设备内存映射内核空间进程内存中，若未实现，用户层调用 mmap()系统将返回 -ENODEV */
　　long (*unlocked_ioctl)(struct file *filp, unsigned int cmd, unsigned long arg);　　
　　　　/* 提供设备相关控制命令（读写设备参数、状态，控制设备进行读写...）的实现，当调用成功时返回一个非负值 */
　　int (*open) (struct inode *, struct file *);　　
　　　　/* 打开设备 */
　　int (*release) (struct inode *, struct file *);　　
　　　　/* 关闭设备 */
　　int (*flush) (struct file *, fl_owner_t id);　　
　　　　/* 刷新设备 */
　　loff_t (*llseek) (struct file *, loff_t, int);　　
　　　　/* 用来修改文件读写位置，并将新位置返回，出错时返回一个负值 */
　　int (*fasync) (int, struct file *, int);　　
　　　　/* 通知设备 FASYNC 标志发生变化 */
　　unsigned int (*poll) (struct file *, struct poll_table_struct *);　　
　　　　/* POLL机制，用于询问设备是否可以被非阻塞地立即读写。当询问的条件未被触发时，用户空间进行select()和poll()系统调用将引起进程阻塞 */
　　...
};
```


## 设备号
Linux内核中使用dev_t类型来描述设备号，这是一个32位整型变量，高12位表示主设备号，低20位表示次设备号，并提供了几个方便操作的宏定义：
```c
typedef u_long dev_t;

#define MAJOR(dev)    ((unsigned int) ((dev) >> MINORBITS))　　// 从设备号中提取主设备号
#define MINOR(dev)    ((unsigned int) ((dev) & MINORMASK))　　// 从设备号中提取次设备号
#define MKDEV(ma,mi)    (((ma) << MINORBITS) | (mi))</span>　　// 将主、次设备号拼凑为设备号
```

#### 分配设备号
- 静态分配设备号
```c
int register_chrdev_region(dev_t from, unsigned count, const char *name);
/* 参数：
　　　　dev_t from - 要申请的设备号（起始）
　　　　unsigned count - 要申请的设备号数量
　　　　const char *name - 设备名
   返回值：
　　　　成功：0
　　　　失败：负数（绝对值是错误码）*/
```

- 动态分配设备号
```c
int alloc_chrdev_region(dev_t *dev, unsigned baseminor, unsigned count, const char *name);
/* 参数：
　　　　dev_t *dev - 用于保存分配到的第一个设备号（起始）
　　　　unsigned baseminor - 起始次设备号
　　　　unsigned count - 要分配设备号的数量
　　　　const char *name - 设备名
   返回值：
　　　　成功：0
　　　　失败：负数（绝对值是错误码）*/
```

这两种分配方式最终调用的是核心函数`__register_chrdev_region()`，执行以下步骤：
- 分配一个`struct char_device_struct`;
- 若设备号范围内的主设备号为0则动态分配一个主设备号，方法是从散列表末尾开始向后搜索一个尚未使用的。
- 初始化其他字段；
- 将新`char_device_struct`插入链表；

## 访问字符设备驱动程序
open()系统调用服务例程中通过适当的文件系统函数，当确定inode与设备文件对应时，则调用`init_special_inode()`，该函数把inode的i_fop字段设置成`def_blk_fops`（块设备）或者`def_chr_fops`（字符设备）文件操作表的地址；接着当调用dentry_open()函数时，分配一个新的文件对象并把f_op字段设置为inode->i_fops；对于字符设备，这个`def_chr_fops`表几乎为空，仅仅定义了`chrdev_open()`函数作为设备文件的打开方法，该函数本质上根据设备号先得到cdev描述符的地址，再使用cdev->ops初始化filp->f_ops，最后调用filp->f_ops->open()执行自定义的open()函数。

## 回顾当时自己写的驱动程序
```c
static int __init _key_init(void){
    printk(banner);

    /* 分配设备号 */
    alloc_chrdev_region(&key_dev.devid, 0, DEVICE_CNT, DEVICE_NAME);
    printk("ana_key major = %d, minor = %d\r\n", MAJOR(key_dev.devid), MINOR(key_dev.devid));

    /* 注册字符设备 */
    cdev_init(&key_dev.cdev, &key_fops);
    cdev_add(&key_dev.cdev, key_dev.devid, DEVICE_CNT);

    /* 创建类 */
    key_dev.class = class_create(THIS_MODULE, DEVICE_NAME);
    if(IS_ERR(key_dev.class)){
        return PTR_ERR(key_dev.class);
    }

    /* 创建设备 */
    key_dev.device = device_create(key_dev.class, NULL, key_dev.devid, NULL, DEVICE_NAME);
    if(IS_ERR(key_dev.device)){
        return PTR_ERR(key_dev.device);
    }

    /* 初始化按键 */
    key_dev.ev_press = 0;
    key_dev.scancode = 0;
    keyio_init();
    return 0;
}
```
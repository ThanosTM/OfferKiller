## 文件系统概述
持久性的数据是存储在外部磁盘上的，如果没有文件系统，访问这些数据需要直接读写磁盘的sector；而文件系统存在的意义就是更有效地组织、管理、和使用磁盘上的这些原始数据。

#### 文件系统的组成
![image](https://pic2.zhimg.com/80/v2-312333f7cbc3cab58d58c464680c0989_720w.jpg)

- 划分粒度：磁盘与内存之间的数据交互的粒度为block，一般为4KB，例如上面一个简单的文件系统管理着64的block的一个磁盘区域。
- User Data：文件本质上就是一些字节构成的集合，上面的文件系统中使用了56个block用于存放用户数据。
- Inode：包含了该文件的相关控制信息，例如访问权限、大小、创建时间等；
- bitmap：记录Inode使用情况的bitmap和数据block使用情况的bitmap；
- 超级块superblock：包含了一个文件系统的所有控制信息，例如有多少inodes，有多少数据block，inode信息起始与哪个block等等。

#### 文件寻址
这5个block中的inodes构成了一个inode table，要访问一个文件就要先访问其文件控制信息inode；
- 对于ext2/3/4文件系统，可以通过`dumpe2fs`来查看其inode bitmap, data block bitmap, inode table等信息；
- 如果只需查看inode的使用情况，则直接使用`df -i`即可；

文件寻址的两种方式：
- ==multi-level index==：类似于一级页表或多级页表管理，这种只使用block指针的方式被ext2/3文件系统所采用，但存在一个问题，对于各种大小的文件，都需要较多的block用于寻址，而实际应用中，大多数文件体积都很小，meta data相对占的比重就很大了。
- `extent`：采用一个block指针加上一个length来表示一组物理上连续的blocks，一个文件由若干的extents表示，这样小文件所需要的Meta data就较少，实现更为灵活，被ext4文件系统所采用：
```c
struct ext4_extent {
    __le32  ee_block;   /* first logical block extent covers */
    __le16  ee_len;     /* number of blocks covered by extent */
    ...
};
```

## VFS概述
![image](https://img-blog.csdn.net/20180318233241252?watermark/2/text/Ly9ibG9nLmNzZG4ubmV0L3UwMTA0ODc1Njg=/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)

VFS，Virtual Filesystem Switch，虚拟文件系统转换，是一个内核软件层，在具体文件系统上抽象的一层，用来处理与POSIX文件系统接口相关的所有调用，表现为给各种不同的文件系统提供一个通用的接口。

#### 结构
![image](https://img-blog.csdn.net/20160527001450432?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQv/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/Center)

#### 统一文件模型Common file model
VFS的核心设计：统一文件模型，定义了4中对象：
- superblock：存储文件系统基本的元数据，如文件系统的类型、大小、状态等等；
- inode：保存一个文件相关的元数据，包括文件的所有者（用户、组），访问时间、文件类型等等，但不包括这个文件的名称！文件和目录都有具体的inode与之对应；
- dentry：保存了文件/目录名称与具体inode的对应关系，同时可以实现目录与其包含的文件之间的映射关系，另外也作为缓存的对象，缓存最近最常访问的文件或者目录，以加速访问；
- file：一组逻辑上相关联的数据，被进程打开并关联使用。

## Superblock
- 静态：superblock保存了一个文件系统最基础的元信息，一般会保存在底层存储设备的开头；
- 动态：挂载后会读取文件系统的superblock并常驻内存，部分字段是临时设置的。

![image](https://img-blog.csdn.net/20180318233329641?watermark/2/text/Ly9ibG9nLmNzZG4ubmV0L3UwMTA0ODc1Njg=/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)

- `s_list`：Linux支持同时挂载多个文件系统，s_list用于构建superblock链表，`sb_lock`用于保护链表不被多处理器同时访问；
- `s_fs_info`：指向属于具体文件系统的超级块信息，例如EXT2系统就会指向`ext2_sb_info`结构体；
- `s_dirt`：分配和释放磁盘块的操作在VFS中可以直接对内存超级块的`s_fs_info`进行操作（其中含有磁盘分配位图），而无需访问磁盘；但这就导致了VFS超级块与磁盘上对应的可能不同步，有必要引入`s_dirt`标志来表示该超级块是否需要同步，Linux是通过周期性地将所有脏超级块写回磁盘来减少不同步所带来的危害。
- ==`s_op`==：包含了所有超级块操作，是最重要的字段之一；该指针指向`struct super_operation`结构体，其中是每个操作函数的函数指针（通过函数指针的方式实现了面向对象的设计思想），其中未实现的方法字段置为NULL；例如当要执行`read_inode()`时：
```c
    sb->s_op->read_inode(inode);
```


## Index Node
文件系统处理文件所需要的所有信息都放在索引结点Index Node的数据结构中，文件名可以随时更改，但inode对于文件是唯一的，并且随文件的存在而存在。

![image](https://img-blog.csdn.net/20180318233344283?watermark/2/text/Ly9ibG9nLmNzZG4ubmV0L3UwMTA0ODc1Njg=/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)

每个索引结点出现在以下位置：
- 某个双向循环链表中（相邻的元素存放在`i_list`中）：有效未使用的inode链表（未被任何进程使用）、正在使用的inode链表（当前正在被一个或读个进程使用）、脏inode链表
- 每文件系统的双向循环链表中：链表头放在超级块`s_inodes`字段中；
- `inode_hashtable`散列表中：加快了对索引结点对象的搜索；

比较重要的字段有：
- `i_dentry`：指定当前inode标识的文件对象的名称，由一条链表组织起来，因为可能有多个dentry指向这个inode(硬链接)；
- `i_mode`：文件类型与访问权限；
- `i_size`：文件的字节数；
- `i_atime`/`i_mtime`/`i_ctime`：上次访问文件时间、上次写文件时间、上次修改索引结点时间；
- `i_op`：索引结点操作，指向`inode_operations`结构体，同样包含一组函数指针。支持的操作有：

1. `create()`：创建一个新的普通文件的inode；
2. `lookup()`：为包含在一个目录项对象中的文件名对应的inode查找目录；
3. `link()`：创建一个硬链接；
4. `mkdir()`：创建一个新的目录的inode；


## File
![image](https://img-blog.csdn.net/20160526230846663?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQv/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/SouthEast)

文件对象描述进程如何与一个打开的文件进行交互，它是在文件被打开时创建的，在磁盘上没有对应的映像。
```c
struct file {
    struct dentry *f_dentry;
    struct vfsmount *f_vfsmnt;
    struct file_operations *f_op;
    mode_t f_mode;
    loff_t f_pos;
    struct fown_struct f_owner;
    unsigned int f_uid, f_gid;
    unsigned long f_version;
    ...
}
```

比较重要的字段有：
- `f_flags`：文件打开时设定的标志；
- `f_mode`：进程的访问模式；
- `f_pos`：当前的文件访问指针，这个指针放在file而不是放在inode中是因为存在多进程同时访问同一文件的情况；
- `f_count`：引用计数器，记录适用文件对象的进程数（以CLONE_FILES标志创建的轻量级进程共享打开的文件表），同时内核本身适用到该文件对象时也要增加引用值；
- `f_op`：指向`file_operation`结构体，包含文件操作集合，例如：

1. `llseek()`：更新文件访问指针；
2. `read(file, buf, count, offset)`：从文件 *offset处开始读出count个字节到buf中，然后增加 *offset的值；
3. `write()`；
4. `ioctl(inode, file, cmd, arg)`：向一个基本硬件设备发送命令，该方法只适用于设备文件；

当VFS为进程打开一个文件时，将调用`get_empty_filp()`函数来分配一个新的文件对象，在该函数中调用`kmem_cache_alloc()`从slab高速缓存分配器`filp_cachep`获得一个空闲的文件对象，并做相应初始化。


## Directory Entry
- dentry用于记录具体的文件名与对应的inode间的对应关系，同时可以实现硬链接、缓存、多级目录等树状文件系统的特性。
- 每个文件系统拥有一个没有父dentry的根目录，该dentry被超级块所引用，作为树形结构的查找入口；其余所有dentry都有唯一的父dentry，并可以若干个孩子dentry。
- 每个dentry都有一个inode对应，但是没有在磁盘等底层持久化存储设备上存储。
- dentry创建后会被操作系统进行缓存，目的是提升OS对文件系统进行操作的性能。

#### 目录项对象结构体
![image](https://img-blog.csdn.net/20180318233353639?watermark/2/text/Ly9ibG9nLmNzZG4ubmV0L3UwMTA0ODc1Njg=/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)

几个关键字段：
- `d_inode`：指向当前dentry关联的inode；
- `d_op`：指向dentry_operations结构体；

dentry在需要时被动态创建，且能被缓存，可以处于以下四种状态之一：
- 空闲free：不包含有效信息，还没有被VFS使用；
- 未使用unused：还没有被内核使用，d_count=0，但d_inode指向关联inode，包含有效信息，但可能会被回收；
- 正在使用in use：d_count > 0；
- ==负状态negative==：关联的inode不复存在，有可能被删除或本来就不存在，d_inode=NULL；

#### 目录项高速缓存
引入目录项高速缓存的目的：从磁盘读入一个目录项并构造相应的目录项对象需要画肺大量的时间，并且完成过对目录项的操作后很有可能不久后还要使用。其包含3个部分：
- LRU双向链表：最近最少使用，所有未使用unused状态的目录项存放于此，链表按照插入的时间排序，最后释放的总是放在链表的首部，最近最少使用的总是靠近尾部；当目录项高速缓存变小，则先从尾部开始删除结点。
- 正在使用in use的双向链表：由inode的`i_dentry`指向（因为每个inode可能被多个硬链接关联），当最后一个硬链接被删除后，该in use的目录项可能变成负negative状态，此时该目录项被移动至LRU链表中，随着每次目录项高速缓存的缩减，该对象实际上是在逐渐向尾部移动，由此达到了逐渐释放的功能。
- 散列表`dentry_hashtable`：从中可以快速获取与给定文件名和目录名对应的目录项对象。当访问的对象不在目录项cache中则返回空值。


## 与进程相关的文件
#### fs_struct
每个进程都有自己的当前工作目录和自己的根目录，这是内核用来表示进程与文件系统相互作用所必须维护的信息，使用`fs_struct`结构体进行描述：
- `struct dentry *root`：根目录的目录项；
- `struct dentry *pwd`：当前工作目录的目录项；

#### files_struct
`files_struct`用于描述一个进程当前打开的文件，该表被PCB的files字段所指向，主要的字段有：
- `fd`指向文件对象的指针数组，数组的索引就是文件描述符fd，通常fd[0]为标准输入文件，fd[1]为标准输出文件，fd[2]为标准错误文件。允许数组中的多个元素指向同一个文件对象，例如用户使用shell结构（2 > &1）将标准错误==重定向==到标准输出文件上。


## 文件系统类型与处理
VFS支持的文件系统可以划分为三种类型：
- 磁盘文件系统：例如EXT2/3/4，VFAT等等；
- 网络文件系统：允许轻易地访问属于其他网络计算机的文件系统所包含的文件，NFS便是一种著名的网络文件系统；
- 特殊文件系统：这些文件系统不管理本地或者远程的磁盘空间，例如/proc；

#### 特殊文件系统
可以为系统程序员和管理员提供一种容易的方式来操作内核的数据结构并实现操作系统的特殊特征。例如：
- proc：对内核数据结构的常规访问点；
- rootfs：为启动阶段提供的一个空的根目录；
- sockfs：套接字

这些特殊的文件系统不限于物理块设备，但是内核会为每个特殊文件系统分配一个虚拟的块设备，其主设备号为0，次设备号为任意值。

#### 文件系统类型注册
文件系统有两种方式被注册，一种是编译内核时就把需要识别的文件系统静态编译进去，一种是作为一个模块动态加载，无论哪种方式都需要对目前已在内核中的所有文件系统进行跟踪，这是通过“注册”来实现的。每个文件系统都可以由`struct file_system_type`来描述，重要的字段有：
- `fs_supers`：指向具有相同文件系统类型的超级块对象链表头；
- `get_sb`：指向依赖于具体文件系统的函数，该函数作用是分配一个超级块对象并初始化；

对于静态编译的文件系统，在系统初始化期间会调用`register_filesystem()`以此注册，并把对应的file_system_type结构体插入链表；对于动态加载的模块，也会调用该函数，这种情况下在卸载时会调用`unregister_filesystem()`；

#### 命名空间namespace
每个进程可以拥有自己的已安装文件系统树——进程的命名空间，通常绝大多数进程共享同一个命名空间，因为都是由init进程clone而来的；当一个进程安装或者卸载一个文件系统时，仅仅修改其命名空间，只对有处于同一命名空间的进程是可见的。【==命名空间的理解？？？==】

命名空间由结构体`struct namespace`描述，其中`struct vfsmount *root`指向已安装文件系统的根。


#### 文件系统安装
Linux中可以通过mount命令安装一个文件系统，umount命令卸载一个文件系统。 安装的文件系统形成一个层次：一个文件系统的mount点可能成为第二个文件系统的目录，等等。Linux使用`struct vfsmount`来跟踪已安装的文件系统（注意与文件系统类型注册的`file_system_type`区分，该结构体针对类别），其关键的字段有：
- `mnt_mountpoint`：该文件系统的挂载点；
- `mnt_namepsace`：指向安装了文件系统的命名空间

#### 安装普通文件系统
mount()系统调用用于安装一个普通文件系统，其服务例程为sys_mount()，其中调用do_mount()进行真正的安装操作，其调用流程为：
![image](https://imgconvert.csdnimg.cn/aHR0cHM6Ly91cGxvYWQtaW1hZ2VzLmppYW5zaHUuaW8vdXBsb2FkX2ltYWdlcy8xNzM1MjQwNS1iMTEzMWIxMmM0ZTk3Y2NmLnBuZz9pbWFnZU1vZ3IyL2F1dG8tb3JpZW50L3N0cmlwfGltYWdlVmlldzIvMi93LzEyMDAvZm9ybWF0L3dlYnA?x-oss-process=image/format,png)

核心的流程为：
- `get_fs_type()`：得到fs类型，有了该类型就能得到其`get_sb()`方法；
- `alloc_vfsmnt()`：分配一个vfsmount描述符；
- `get_sb()`：依赖具体文件系统实现，分配并初始化一个超级块；
- 字段的一系列初始化，主要是一些数据结构之间的联系关系，这样这个文件系统就暴露给内核了。

#### 安装根文件系统
安装根文件系统是内核初始化的关键步骤，其复杂性来源于允许根文件系统存放在磁盘、NFS、ramdisk等不同地方（uboot启动参数可以指定并传递给内核）。安装根文件系统分为两个阶段：
- 安装特殊rootfs文件系统，该文件系统仅提供一个作为初始安装点的目录；
- 安装实际根文件系统；

###### 为何要分两步？
rootfs文件系统允许内核容易地改变实际根文件系统的位置，例如，内核可以先把一个最小的文件系统作为根安装，随后开始探测系统硬件，装入必需的内核模块，并从物理快设备安装实际的根文件系统。

###### 安装rootfs文件系统
由`init_rootfs()`和`init_mount_tree`完成，使用""rootfs"传入`do_kern_mount`函数，完成安装操作。

###### 安装实际根文件系统
在内核初始化即将结束时进行，根据内核编译时所选择的选项和启动选项决定。安装结束后将rootfs文件系统根目录上已安装的文件系统安装点移动，此时rootfs并没有被卸载，而是被隐藏了。（==？？？==）

## 按路径名查找
进程需要识别一个文件时，其本质就是完成从文件名到对应的文件inode的转换，由`path_lookup()`函数完成，其中要考虑以下：
- 将路径名拆分成一个文件名序列，除了最后一个文件名外其余文件名必定都是目录。
- 第一个字符为'/'则为绝对路径，从current->fs->root开始查找，否则为相对路径，从current->fs->pwd开始；
- 检查第一个名字匹配的目录项以获得相应inode，并从磁盘中读出包含该inode的目录文件，并对其中每个名字检查与第二个名字匹配的目录项以获得inode，如此重复；
- 目录项高速缓存的存在大大加速了该过程，很多情况下可以避免从磁盘读取中间目录。
- 考虑访问权限检查；
- 考虑符号链接的扩展，并防止循环引用（嵌套层数不超过5）；
- 考虑延伸到新的文件系统；
- 应该完全在该进程的命名空间中完成。

其查找结果存放在`struct nameidata`中：
- `dentry`指向最后一个路径分量的目录项对象；
- `mnt`指向其以安装文件系统对象；

其调用过程为：
- `path_lookup()`先进行一些预先准备；
- `link_path_walk()`进行真正的查找操作；
- 对于每层循环，对于每个分量，执行：权限检查、探测下个分量（这里要区分"."和".."和其他）
- `do_lookup()`：先去`__d_lookup()`到目录项高速缓存中搜索，没有再去`real_lookup()`从磁盘读取目录，总之将得到这次循环解析到的分量名的目录项dentry和已安装的文件对象mnt。
- 检查是否是另一个文件系统的挂载点，是否是符号链接，是否是目录；
- 返回最终的查找结果；


## ==VFS系统调用的实现==
#### open()系统调用
open()系统调用的服务例程为sys_open()函数，他将要接收打开文件的路径名、访问模式的flags、所需要的许可权掩码mode。如果调用成功则返回文件描述符fd，若失败则返回-1.

```c
//open系统调用过程
SYSCALL_DEFINE3(open, const char __user *, filename, int, flags, umode_t, mode)
--->>> do_sys_open(dfd, filename, flags, mode);
    --->>>do_filp_open(dfd, tmp, &op, lookup);
        --->>>path_openat(dfd, pathname, &nd, op, flags | LOOKUP_RCU);
            --->>>do_last(nd, &path, op, pathname);
                --->>>filp = nameidata_to_filp(nd);
                    --->>>__dentry_open(nd->path.dentry, nd->path.mnt, filp,
				     NULL, cred);
                     --->>>if (!open && f->f_op)
                    		open = f->f_op->open;
                    	if (open) {
                    		error = open(inode, f);
```

大致流程：
- `getname()`：从进程地址空间读取文件路径名；
- `get_unused_fd()`：从当前进程PCB的打开文件fd数组中找到一个空的位置用于存放新的fd；
- `open_namei()`：根据路径名查找得到nameidata的数据结构的查找结果（见上）；
- `dentry_open()`：分配一个新的文件对象，设置好字段，特别是`f_op`设置为dentry->d_inode->i_fop字段的内容，再调用具体文件的open方法，最后返回文件对象的地址；
- 返回fd；

#### read()/write()系统调用
read()与write()系统调用非常相似，下面仅以read()为例。read()接收3个参数，文件描述符fd，一个内存区的地址buf，以及一个数count，返回值为成功传输的字节数，若发生错误则返回-1.（返回值小于给定的count并不意味发生错误，有几种典型的能够读到小于count的字节数的情形：从管道或设备中断读取、读到文件末尾、被信号中断）；读写操作总是基于文件偏移量`f_pos`，完成读写操作的同时会自动更新该偏移量。

```c
SYSCALL_DEFINE3(read, unsigned int, fd, char __user *, buf, size_t, count)
{
	struct file *file;
	ssize_t ret = -EBADF;
	int fput_needed;

	file = fget_light(fd, &fput_needed);
	if (file) {
		loff_t pos = file_pos_read(file);
		ret = vfs_read(file, buf, count, &pos);
		file_pos_write(file, pos);
		fput_light(file, fput_needed);
	}

	return ret;
}
```

大致流程为：
- `fget_lighe()`：根据fd获取file对象；
- 一些列检查：权限检查、file对象是否有read()方法、粗略检查buf与count、文件的访问部分是否有强制锁等等；
- 调用`file->f_op->read()`；
- 释放文件对象、返回实际传送的字节数；

#### close()系统调用
close()系统调用接收的参数只有一个，待关闭的文件描述符fd，其大致流程为：
- 将current->files->fd[fd]置位NULL，释放fd，这是通过清除current->files中的一些字段进行的；
- 调用filp_close()：调用文件操作的flush()方法、释放锁、释放文件对象
- 返回0或者出错码：flush()或者前一个写操作可能产生错误码；


## 简单了解文件加锁
该问题来源于多个进程同时对一个文件进行读写所涉及的同步问题，根据POSIX标准，进程可以选择对文件的一部分甚至一个字节进行加锁；
- 劝告锁：不限制内核访问文件，而是靠着协作进程间合作检查锁的存在而实现文件同步；
- 强制锁：Linux还引入了强制锁，默认是不开启的，内核在进行read()、open()等系统调用时都会检查，强制锁的存在将使得这些操作失败。

进程可以通过以下两个方式获得或释放一个文件劝告锁：
- `flock()`系统调用，基于FL_FLOCK锁，接收fd和锁操作命令，应用于整个文件；
- `fcntl()`系统调用，基于FL_POSIX锁，接收fd、锁操作命令、flock指针，允许对文件部分加锁；

#### 【TODO：实现】


## 两种Link：硬链接与软链接
![image](https://pic4.zhimg.com/80/v2-47287d5c036d5c79e4350f3448924ee7_720w.jpg)

#### 硬链接
硬链接通过索引结点来进行连接，Linux中多个文件名指向同一inode是允许的，硬链接的作用在于允许一个文件拥有多个文件名，以防止误删，删除一个连接并不会删除inode，只有所有连接删除后，inode才会被真正删除。

#### 软链接
也叫符号链接，实际上是一个特殊的文本文件，其中包含另一个文件的位置信息。其inode与真正指向的文件inode是不同的。

#### 摘录自网上的实验
- 分别对同一个文件创建两种link：硬链接为"ln"，符号链接为"ln -s"
![image](https://pic3.zhimg.com/80/v2-3924b3d6d042b5291b25b50bc581c57e_720w.png)

- 使用"cat"命令可以发现输出的内容是一样的：
![image](https://pic2.zhimg.com/80/v2-4e00deb974efa6605c1fb4c44ae50c25_720w.jpg)

- 但是inode却不相同，文件信息也不相同：
![image](https://pic1.zhimg.com/80/v2-34c4b0e7202fcbf50f196263c4107f08_720w.png)

![image](https://pic2.zhimg.com/80/v2-4dcbecae63d96f24429efd92d624db71_720w.png)

- 若此时删除掉真正的文件，硬链接还能够正常访问，因为真实的inode还存在并被硬链接所指向，而软链接已经访问失败了，因为所记录的位置处已经无该文件了。




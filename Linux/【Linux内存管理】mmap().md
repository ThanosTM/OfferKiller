## 概述
`mmap()`能够将文件映射到内存空间，然后通过读写内存来读写文件，其函数声明如下：
```c
void *mmap(void *start, size_t length, int prot, int flags, int fd, off_t offset);
```

参数说明：
- start：指定要映射的内存地址，一般设置为 NULL 让操作系统自动选择合适的内存地址。
- length：映射地址空间的字节数，它从被映射文件开头 offset 个字节开始算起。
- prot：指定共享内存的访问权限。可取如下几个值的可选：PROT_READ（可读）, PROT_WRITE（可写）, PROT_EXEC（可执行）, PROT_NONE（不可访问）。
- flags：由以下几个常值指定：MAP_SHARED（共享的） MAP_PRIVATE（私有的）, MAP_FIXED（表示必须使用 start 参数作为开始地址，如果失败不进行修正），其中，MAP_SHARED , MAP_PRIVATE必选其一，而 MAP_FIXED 则不推荐使用。
- fd：表示要映射的文件句柄。
- offset：表示映射文件的偏移量，一般设置为 0 表示从文件头部开始映射。

==mmap只是在虚拟内存分配了地址空间，只有在第一次访问虚拟内存的时候才分配物理内存。==

## `mmap()`底层原理
调用 `mmap()` 时，内核会创建一个 `vm_area_struct` 结构，并且把 vm_start 和 vm_end 指向虚拟内存空间的某个内存区，并且把 vm_file 字段指向映射的文件对象。然后调用文件对象的 mmap 接口来对 vm_area_struct 结构的 vm_ops 成员进行初始化。对映射到内存空间的读写不会实时写入文件，而是对文件缓存进行操作，也可以显式地调用`msync()`来实现内存与文件之间的同步。

![image](https://pic2.zhimg.com/80/v2-a56c211119318477a016cf8d2cb533b1_720w.jpg)
![image](https://pic4.zhimg.com/80/v2-85ce9e112b1598974a32f8febe0cf7cb_720w.jpg)

## 优缺点
优点：
- 对文件的读取操作减少了数据的拷贝次数（从内核拷贝到用户态），用内存读写取代IO读写，提高了文件读取效率
- 提供了进程间共享内存和相互通信方式：映射到同一文件或匿名映射到同一区域

缺点
- 对于小文件不适用，因为内存映射最小粒度为页，将造成大量浪费
- 变长文件不适合，因为映射的空间无法完成拓展
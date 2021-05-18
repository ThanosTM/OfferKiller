## 1、Linux虚拟地址空间如何分布，32位和64位有何不同？
#### Linux虚拟地址空间分布
由低地址到高地址分别为：
- 只读段：包括代码段、rodata段（常量字符串、#define）
- 数据段：保存全局变量、静态变量
- 堆：动态内存，由malloc/new分配，堆顶的位置可以通过函数brk()、sbrk()进行动态调整，向高地址增长；
- 文件映射区：动态链接库、共享内存等映射物理空间的内存，一般是由mmap()分配
- 栈：维护函数函数调用上下文、局部变量的空间，向低地址增长，一般为8M，可以通过ulimit -s查看。
- 内核虚拟空间：用户代码不可见，由内核管理

![image](https://pic3.zhimg.com/80/v2-0e174cd7632d05717437f49e94ae60be_720w.png)

#### 64位系统
==64位系统实际只有2^48大小的虚拟地址空间==，对应着256TB，因为并不需要那么大的寻址空间形成浪费。64位Linux一般使用48位表示虚拟地址空间，40位物理地址空间，这可以通过`/proc/cpuinfo`查看


## 2、malloc是如何分配内存的
malloc()是glibc中内存分配的函数，也是最常用的动态内存分配函数，其内存必须通过free()来进行释放，否则会导致内存泄露。关于malloc()的具体实现，与glibc的版本有关，大体为：
- 若分配内存小于128K，则调用sbrk()，将堆顶指针向高地址移动，获得新的虚存空间
- 若分配内存大于128K，则调用mmap()，在文件映射区分配匿名虚存空间
- 128K分界是glibc的默认设置，这可以通过mallopt()函数进行修改


## 3、malloc分配多大内存，就占用多大的物理内存空间吗？
由上述讨论可知，malloc()分配的实际上是虚拟内存，而虚拟内存和物理内存通过进程页表进行映射，这两者均可以使用`ps aux`指令进行查看，关注第五、六列。
- VSZ，virtual memory size，进程总共使用的虚拟地址空间大小
- RSS，resident set size，进程实际使用的物理内存大小

可以编写以下程序进行验证：
```c
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <malloc.h>
char ps_cmd[1024];
void print_info(
        char*      var_name,
        char*      var_ptr,
        size_t     size_in_kb
)

{
        printf("Address of %s(%luk) 0x%lx,  now heap top is 0x%lx\n",
                 var_name, size_in_kb, var_ptr, sbrk(0));
        system(ps_cmd);
}

int main(int argc, char** argv)
{
        char *non_set_var, *set_1k_var, *set_5k_var, *set_7k_var;
        pid_t pid;
        pid = getpid();
        sprintf(ps_cmd, "ps aux | grep %lu | grep -v grep", pid);
        non_set_var = malloc(32*1024);
        print_info("non_set_var", non_set_var, 32);
        set_1k_var = malloc(64*1024);
        memset(set_1k_var, 0, 1024);
        print_info("set_1k_var", set_1k_var, 64);
        set_5k_var = malloc(127*1024);
        memset(set_5k_var, 0, 5*1024);
        print_info("set_5k_var", set_5k_var, 127);
        set_7k_var = malloc(64*1024);
        memset(set_1k_var, 0, 7*1024);
        print_info("set_7k_var", set_7k_var, 64);
        return 1;
}
```

思路是：每次malloc后均使用`ps aux`获取进程的内存使用信息，再使用memset函数后，再次获取信息。结果如下：

- VSZ不是每次malloc()后都会增长，即堆顶没有发生变化，这是因为malloc尝试重用堆顶剩余空间，这样的malloc是轻量快速的。
- RSS增量很少，不是每次malloc都会马上分配实际的物理内存，只有当第一次使用时，例如memset，才会分配。
- RSS的增量总是4K的倍数，因为物理页面大小为4K

结论：==不是malloc()后马上就会分配实际内存，而是第一次使用时发现还未分配实际内存而产生缺页中断，才分配实际的页面，同时更新进程页表信息，这是Linux虚拟内存管理的核心之一==。


## 4、如何查看进程虚拟地址空间的使用情况？
Linux提供pmap命令来查看进程虚拟空间地址的使用情况，命令格式为`pmap -d $pid`，打印结果为：
```c
17867: test_mmap

START       SIZE     RSS   DIRTY PERM OFFSET   DEVICE MAPPING

00400000      8K      4K      0K r-xp 00000000 08:01  /home/mysql/vin/test_memory/test_mmap

00501000     68K      8K      8K rw-p 00001000 08:01  /home/mysql/vin/test_memory/test_mmap

00512000     76K      0K      0K rw-p 00512000 00:00  [heap]

0053e000    256K      0K      0K rw-p 0053e000 00:00  [anon]

2b3428f97000    108K     92K      0K r-xp 00000000 08:01  /lib64/ld-2.4.so

2b3428fb2000      8K      8K      8K rw-p 2b3428fb2000 00:00  [anon]

2b3428fc1000      4K      4K      4K rw-p 2b3428fc1000 00:00  [anon]

2b34290b1000      8K      8K      8K rw-p 0001a000 08:01  /lib64/ld-2.4.so

2b34290b3000   1240K    248K      0K r-xp 00000000 08:01  /lib64/libc-2.4.so

2b34291e9000   1024K      0K      0K ---p 00136000 08:01  /lib64/libc-2.4.so

2b34292e9000     12K     12K     12K r--p 00136000 08:01  /lib64/libc-2.4.so

2b34292ec000      8K      8K      8K rw-p 00139000 08:01  /lib64/libc-2.4.so

2b34292ee000   1048K     36K     36K rw-p 2b34292ee000 00:00  [anon]

7fff81afe000     84K     12K     12K rw-p 7fff81afe000 00:00  [stack]

ffffffffff600000   8192K      0K      0K ---p 00000000 00:00  [vdso]

Total:    12144K    440K     96K
```


## 5、free的内存真的释放了吗？
实验示意图：
![image](https://mc.qcloudimg.com/static/img/fc9056547d6a26099642a89b82c67f6d/image.jpg)
![image](https://mc.qcloudimg.com/static/img/e08d098b86422a515d8f206b07be6e79/image.jpg)

结论：
- 当malloc使用mmap()分配内存时（大于128K），free会调用munmap()马上返还给OS，实现真正的释放
- 堆内的内存，只有释放堆顶的空间，同时堆顶连续空闲空间大于128K才会使用sbrk(-SIZE)调整堆顶指针的位置，真正归还给OS
- 堆内部的空闲空间，是不会返还给OS的，形成了内存空洞。

故：随着系统频繁使用malloc/free，堆内会出现越来越多不可用的碎片，导致内存泄露，而这种内存泄露是使用类似valgrind工具无法检测出来的。因此更好的方式是，不能完全依赖glibc的malloc/free，而是==建立属于进程的内存池==，即一次性分配一大块内存，小内存从内存池中获得，当进程结束或内存不可用时，一次性释放掉，大大减小碎片的产生。


## 6、既然堆内存不能直接释放，为什么不能全部使用mmap()来分配？
- 频繁地使用系统调用sbrk()/mmap()/munmap()开销较大；并且mmap()申请的内存被munmap()后重新申请会导致更多的缺页中断；如果使用mmap()分配小内存，会导致地址空间的碎片更多。

- 而堆是一个连续的空间，堆内的碎片还没有归还OS，如果可以进行重用，再次访问可能不需要系统调用以及缺页中断，提升了效率。

- 实际上，glibc的malloc实现中，正是充分考虑了sbrk和mmap行为上的差异和优缺点，使用128K作为默认分界线进行分治。


## 7、如何查看进程的缺页中断信息
Linux中可以使用一下命令进行查看：
```
ps -o majflt,minflt -C <program_name>
ps -o majflt,minflt -p <pid>
```

- 这两个数值表示一个进程自启动以来所发生的缺页中断次数。majflt与minflt不同在于：majflt表示需要读写磁盘，可能是内存对应的页面在磁盘中需要读入，也可能是物理内存不足需要淘汰部分页面至磁盘中。

- 如果进程的内核态CPU使用过多，应该考虑是否为缺页次数过多，如果majflt过大，表明内存不足，minflt过大则很有可能频繁分配和释放了大块内存，这时可以考虑增大malloc的临界值，或者程序实现内存池。


## 8、如何查看堆内存的碎片情况？
glibc提供了以下结构和接口来查看堆内内存和mmap的使用情况：
```c
struct mallinfo {
  int arena;    /* non-mmapped space allocated from system */
  int ordblks;  /* number of free chunks */
  int smblks;   /* number of fastbin blocks */
  int hblks;    /* number of mmapped regions */
  int hblkhd;   /* space in mmapped regions */
  int usmblks;  /* maximum total allocated space */
  int fsmblks;  /* space available in freed fastbin blocks */
  int uordblks; /* total allocated space */
  int fordblks; /* total free space */
  int keepcost; /* top-most, releasable (via malloc_trim) space */
};


/* 返回 heap(main_arena) 的内存使用情况，以 mallinfo 结构返回 */
struct mallinfo mallinfo();
/* 将 heap 和 mmap 的使用情况输出到 stderr  */
void malloc_stats();
```
想要知道堆内碎片到底有多碎，重点关注`fsmblks` 、 `smblks` 、 `ordblks`这三个值，分别对应了0~80字节，80~512字节，512~128K字节，如果前两个值过大，则说明堆内碎片的问题就很严重了。


## 9、除了glibc的mallac/free，还有其他第三方实现吗？
glibc的内存管理在高并发下性能低下，内存碎片化问题也很严重，陆续有一些第三方工具来替换，最著名的有==google的tcmalloc==和==facebook的jemalloc==.

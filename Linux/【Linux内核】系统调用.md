## 区分系统调用与POSIX API
- 系统调用：实现了用户态进程和硬件设备之间的大部分接口，是通过软中断向内核态发出的一个明确的请求。
- API：函数定义，说明了如何获得一个给定的服务，判断一个系统是否与POSIX系统兼容要看它是否提供了一组合适的应用程序接口，而不管其具体实现；
- 从编程者角度看，API和系统调用时没有差别的；但对于内核设计者而言，系统调用属于内核，而用户态的库函数不属于内核。

## x86架构的系统调用流程
#### 进入系统调用
- 执行`int 0x80`指令，这在老版本的Linux内核中这是用户态进入内核态的唯一方式；
- 执行`sysenter`指令，这是Linux2.6开始支持的指令，也叫“快速系统调用”，由于int指令要执行几个一致性和安全检查，所以速度较慢；

#### 执行系统调用
- 在内核初始化期间，0x80号中断被初始化为系统调用：
```c
    set_system_gate(0x80, &system_call);
```
- 当用户进程发出`int 0x80`指令的时候，CPU切换到内核态并开始执行system_call地址出的指令；
```c
system_call:
    pushl %eax
    SAVE_ALL
    movl 0xffffe000, %ebx
    andl %esp, %ebx
```
该函数将系统调用号%eax，和异常处理程序用到的寄存器保存到栈中（%ebx, %ecx, %edx, %esi, %edi, %ebp）
- 调用号有效性检查：
```c
    cmpl $NR_syscalls, %eax
    jb nobadsys
    movl $(-ENOSYS), 24(%esp)
    jmp resume_userspace
nobadsys:
```
- 最后调用%eax对应的特定服务例程：
```c
    call *sys_call_table(0, %eax, 4)
```
这里4代表分配表每个表项占4个字节，因此调用号%eax乘上4再加上sys_call_table起始地址就是该系统调用服务例程的指针。

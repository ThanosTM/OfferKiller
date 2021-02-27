## 概述
const声明的变量一般来说一方面供编译器检查，一方面提示程序员这个值不应该被修改；

## C语言修改const类型变量
- 实验1：直接对const变量进行修改
```
#include <stdio.h>

int main(void)
{
    const int a = 5;
    a = 10;
    printf("a=%d",a);  
}
```
结果：编译报错！

- 实验2：通过指针间接修改
```c
#include <stdio.h>

int main(void)
{
    const int i = 5;
    int *p = (int *)&i;   
    *p = 10;

    printf("&i=0x%x \r\n",&i);
    printf("p=0x%x \r\n",p);
    printf("i=%d \r\n",i);
    printf("*p=%d \r\n",*p);
    
    return 0;
}
```
结果：const的值竟然成功被修改

- 实验3：间接修改const全局变量
```c
#include <stdio.h>
const int i = 5;
int main(void)
{
    int *p = (int *)&i;       
    *p = 10;
  
    printf("&i=0x%x \r\n",&i);
    printf("p=0x%x \r\n",p);
    printf("i=%d \r\n",i);
    printf("*p=%d \r\n",*p);
    
    return 0;
}
```
结果：段错误！

- 解释：==const全局变量存储在只读数据段==，编译期最初将其保存在符号表中，第一次使用时为其分配内存，在程序结束时释放。
- 下文补充C/C\+\+程序中数据存储位置分析知识

## C++中const类型变量的修改

首先明确：==C++标准中规定对const类型的值的修改是未定义行为==，结果可能取决于不同编译器的实现，因此不建议这么做！

- 实验1：指针间接修改const类型的值

```cpp
#include <iostream>
using namespace std;

int main(){
    const int i = 2;
    int *a = (int *)&i;
    cout << *a << endl;
    *a = 3;
    cout << *a << ' ' << a << ' '<<  i << ' ' << &i << endl;
    return 0;
}
```
结果：
```
2
3 0x61fe14 2 0x61fe14
```

那么这就很奇怪了，到底这算不算成功修改了？为什么相同地址处的值却不同？

通过查看生成的汇编代码可知：==i是直接从编译器符号表中输出==的，即此处编译器进行了优化，它不认为const的值会发生更改，因此直接从符号表中取出立即数进行赋值，而真实内存0x61fe14处的值已经被修改。

- 实验2：加volatile

```cpp
#include <iostream>
using namespace std;

int main(){
    volatile const int i = 2;
    int *a = (int *)&i;
    cout << *a << endl;
    *a = 3;
    cout << *a << ' ' << a << ' '<<  i << ' ' << &i << endl;
    return 0;
}
```

结果：
```
2
3 0x61fe14 3 1
```

此时每次i都将重新从内存中读入，表明i已经被修改


## 补充：C/C\+\+程序中数据存储位置分析

```cpp
static int  val_a = 1 ; // 初始化的静态变量 
int  val_b = 2 ;        // 全局变量 
const int  val_c = 3 ;  // const 全局变量 
static int  val_d ;     // 未初始化的静态变量 
int  val_e ;            // 未初始化的全局变量
int main()
{
    static int val_f = 5; //初始化的局部静态变量
    static int val_g;     //未初始化局部静态变量
    int    val_h = 6;      //初始化局部变量
    int    val_i;          //未初始化局部变量
    const  int val_j = 7;  //const局部变量
    return 0;
}
```

1. static无论是全局变量还是局部变量都存储在==全局/静态区域==，在编译期就为其分配内存，在程序结束时释放，例如：val_a、val_d、val_h、val_i
2. const全局变量存储在==只读数据段==，编译期最初将其保存在符号表中，第一次使用时为其分配内存，在程序结束时释放，例如：val_c；
3. const局部变量存储在==栈==中，代码块结束时释放，例如：val_j。
4. 全局变量存储在==全局/静态区域==，在编译期为其分配内存，在程序结束时释放，例如：val_b、val_e。
5. 局部变量存储在==栈==中，代码块结束时释放，例如：val_h、val_i。

==在地址分配方面：== 对于堆来讲，向着内存地址增加的方向；对于栈来讲，向着内存地址减小的方向增长。(联系：小尾端是高位字节在高端地址、低位字节在低位地址，因此在压栈时先压高字节后压低字节)。

==分配效率：==
栈是机器系统提供的数据结构，计算机会在底层对栈提供支持：分配专门的寄存器存放栈的地址，压栈出栈都有专门的指令执行，这就决定了栈的效率比较高。堆则是C/C++函数库提供的，它的机制是很复杂的，例如为了分配一块内存，库函数会按照一定的算法（具体的算法可以参考数据结构/操作系统）在堆内存中搜索可用的足够大小的空间，如果没有足够大小的空间（可能是由于内存碎片太多），就有可能调用系统功能去增加程序数据段的内存空间，这样就有机会分到足够大小的内存，然后进行返回。显然，堆的效率比栈要低得多。


## 参考文章
1. https://www.cnblogs.com/ralap7/p/9115646.html
2. https://blog.csdn.net/heyabo/article/details/8745942/
3. https://blog.csdn.net/nyist_zxp/article/details/80257760
4. https://blog.csdn.net/gao1440156051/article/details/51003295
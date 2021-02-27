## 什么是泛型编程？
- 泛型编程（generic programming）是程序设计语言的一种风格或范式。泛型允许程序员在强类型程序设计语言中编写代码时使用一些以后才指定的类型，在实例化时作为参数指明这些类型。
- C\+\+中的泛型编程：模板
```cpp
#include <iostream>
template <class T>
T add(T a,T b){
  T ret = a + b;
  std::cout<< a << " + " << b <<" = " << ret << std::endl;
  return ret;
}
int main(){
  add(1,2);  // 整数相加
  add(1.2,2.3); // 浮点数相加
  return 0;
}
```

## C语言实现一定程度上的泛型编程
#### _Generic关键字（C11支持）
支持泛型表达式
```c
_Generic((value). int:"int", float:"float",char*:"char*",default:"other type")
```

使用示例：
```c
#include <stdio.h>
// int类型加法
int addI(int a, int b)
{
    printf("%d + %d = %d\n",a,b, a + b );
    return (a + b);
}

// double类型加法
double addF(double a, double b)
{
    printf("%f + %f = %f\n",a,b, a + b );
    return (a + b);
}

void unsupport(int a,int b)
{
    printf("unsupport type\n");
}

#define ADD(a,b) _Generic((a), \
    int:addI(a,b),\
    double:addF(a,b), \
    default:unsupport(a,b))
    
int main(void)
{
    ADD(1 , 2);
    ADD(1.1,2.2);
    return 0;
}
```
事实上要定义多种不同类型不同名字的函数，然后将类型作为参数传入，选择对应的函数执行（C\+\+中由编译器完成了这件事）

#### void *指针

最典型的例子：库函数qsort实际上就是泛型排序算法了，它可以针对任何类型的数据进行排序。只需要按照它的协议，实现一个compare函数，用于比较大小。
```c
#include <stdlib.h>
void qsort( void *base, 
            size_t nmemb, 
            size_t size,
            int (*compar)(const void *, const void *)
        );

```

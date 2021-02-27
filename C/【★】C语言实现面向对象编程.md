## C与C++的比较
- 在嵌入式开发中，C/C\+\+语言的使用是最普及的，在C\+\+11版本之前，它们的语法是比较类似的，只不过C\+\+提供了==面向对象==的编程方式。而从C\+\+11开始，C\+\+已经不是当年C语言的扩展了，更像是一门全新的语言。

- C语言的优缺点
1. 优点：简单易学、代码量小、使用者基数大
2. 缺点：可维护性差、扩展性差、可读性差、没有命名空间、数据封装差不安全、开发效率低、可移植性差。

- 更多的C与C\+\+的区别将另开文档记录


## 面向对象的编程思想
- 程序 = 数据结构 + 算法
- 面向过程C：数据结构和算法是分开的
- 面向对象C\+\+：数据结构和算法封装在一起，属性操作、行为调用均通过对象来执行
- 需要解决的3个问题：

==封装、继承、多态==


## 封装的解决
- 定义一个父类Animal，实现数据的封装

Animal.h
```c
#ifndef _ANIMAL_H_
#define _ANIMAL_H_

// 定义父类结构
typedef struct {
    int age;
    int weight;
} Animal;

// 构造函数声明
void Animal_Ctor(Animal *this, int age, int weight);

// 获取父类属性声明
int Animal_GetAge(Animal *this);
int Animal_GetWeight(Animal *this);

#endif
```

Animal.c
```c
#include "Animal.h"

// 父类构造函数实现
void Animal_Ctor(Animal *this, int age, int weight)
{
    this->age = age;
    this->weight = weight;
}

int Animal_GetAge(Animal *this)
{
    return this->age;
}

int Animal_GetWeight(Animal *this)
{
    return this->weight;
}
```

- 与C\+\+的比较：多了一个*this，因为在C\+\+的方法中，隐含了 *this，这里显示定义 *this，并通过其将对象实例作为参数传入。

- 测试方式：定义两个不同的Animal父类，调用接口函数打印各自的参数
```c
#include "animal.h"
#include <stdio.h>

int main(){
    Animal A1, A2;
    Animal_Ctor(&A1, 10, 60);
    Animal_Ctor(&A2, 12, 50);
    printf("A1: %d, %d\n", Animal_GetAge(&A1), Animal_GetWeight(&A1));
    printf("A2: %d, %d\n", Animal_GetAge(&A2), Animal_GetWeight(&A2));
    return 0;
}
```


## 继承的解决
- 从父类Animal中继承一个子类Dog，包含父类所有的成员与方法，并且拥有自己的成员与方法

Dog.h
```c
#ifndef _DOG_H_
#define _DOG_H_

#include "Animal.h"

// 定义子类结构
typedef struct {
    Animal parent; // 第一个位置放置父类结构
    int legs;      // 添加子类自己的属性
}Dog;

// 子类构造函数声明
void Dog_Ctor(Dog *this, int age, int weight, int legs);

// 子类属性声明
int Dog_GetAge(Dog *this);
int Dog_GetWeight(Dog *this);
int Dog_GetLegs(Dog *this);

#endif
```

Dog.c
```c
#include "Dog.h"

// 子类构造函数实现
void Dog_Ctor(Dog *this, int age, int weight, int legs)
{
    // 首先调用父类构造函数，来初始化从父类继承的数据
    Animal_Ctor(&this->parent, age, weight);
    // 然后初始化子类自己的数据
    this->legs = legs;
}

int Dog_GetAge(Dog *this)
{
    // age属性是继承而来，转发给父类中的获取属性函数
    return Animal_GetAge(&this->parent);
}

int Dog_GetWeight(Dog *this)
{
    return Animal_GetWeight(&this->parent);
}

int Dog_GetLegs(Dog *this)
{
    // 子类自己的属性，直接返回
    return this->legs;
}
```

- 这里的一个关键是：要把父类结构放置在子类结构体中的第一个位置！
- 与C\+\+的比较：这里子类构造函数中调用了父类的构造函数，这本身和C\+\+的初始方法是一样的。
- 其中Dog_GetAge也可以强制类型转换实现：
```c
void Dog_GetAge(Dog *me){
    return Animal_GetAge((Animal *)me);
}
```


## 多态的解决
- 虚表与虚函数指针

Animal.h
```
#ifndef _ANIMAL_H_
#define _ANIMAL_H_

#include <assert.h>

struct AnimalVTable;

typedef struct {
    struct AnimalVTable *vptbl;
    int age;
    int weight;
} Animal;

struct AnimalVTable {
    void (*say)(Animal *me);
};

void Animal_Say_(Animal *me){
    assert(0);
}

void Animal_Ctor(Animal *me, int age, int weight){
    me->age = age;
    me->weight = weight;
    static struct AnimalVTable animal_vtbl = {Animal_Say_};
    me->vptbl = &animal_vtbl;
}

int Animal_GetAge(Animal *me){
    return me->age;
}

int Animal_GetWeight(Animal *me){
    return me->weight;
}

void Animal_Say(Animal *me){
    me->vptbl->say(me);
}

#endif
```

Animal.h
```c
#ifndef _DOG_H_
#define _DOG_H_

#include "animal.h"
#include <stdio.h>

typedef struct {
    Animal parent;
    int legs;
} Dog;

void Dog_Say_(Animal *me){
    Dog *dog = (Dog *)me;
    printf("Dog\n");
}

void Dog_Ctor(Dog *me, int age, int weight, int legs){
    Animal_Ctor(&me->parent, age, weight);
    static struct AnimalVTable dog_tbl = {Dog_Say_};
    me->parent.vptbl = &dog_tbl;
    me->legs = legs;
}

int Dog_GetAge(Dog *me){
    //return Animal_GetAge(&me->parent);
    return Animal_GetAge((Animal *)me);
}

int Dog_GetWeight(Dog *me){
    return Animal_GetWeight(&me->parent);
}

int Dog_GetLegs(Dog *me){
    return me->legs;
}

void Dog_Say(Dog *me){
    me->parent.vptbl->say((Animal *)me);
}

#endif
```

- 思想：子类在继承父类之后，在内存中又会开辟一块空间来放置子类自己的虚表，然后让继承而来的虚表指针指向子类自己的虚表
![调用关系](https://img-blog.csdn.net/20180814183452583?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L29ubHlzaGk=/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)

- 测试：
```c
int main()
{
    // 在栈中创建一个子类Dog对象
    Dog d;  
    Dog_Ctor(&d, 1, 3, 4);

    // 把子类对象赋值给父类指针
    Animal *pa = (Animal *)&d;
    
    // 传递父类指针，将会调用子类中实现的虚函数
    Animal_Say(pa);
    
    return 0;
}
```
虽然传入的是Animal *类型的指针，但实际指向的是Dog子类的对象，将调用Dog子类中的虚表中函数指针指向的函数，即Dog\_Say\_()


## 参考文章
1. https://www.cnblogs.com/sewain/p/14164181.html
2. https://blog.csdn.net/onlyshi/article/details/81672279




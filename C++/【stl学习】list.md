## 特性
list实际上是一个双向循环链表，非连续空间存储（因此插入删除节点原迭代器不会失效），优点在于在任何地方插入或者删除节点的时间复杂度都是O(1)，不提供下标随机访问。

##  初始化操作

```
list<Type> a;                                        //定义一个Type类型的链表a
list<Type> a(10);                                    //定义一个Type类型的链表a，并设置初始大小为10
list<Type> a(10, 1);                                 //定义一个Type类型的链表a，并设置初始大小为10，且初始值都为1
list<Type> a(b);                                     //定义并用链表b初始化链表a
int array[] = {1, 2, 3, 4, 5};
list<int> a(array, array+5);                         //将数组n的前5个元素作为链表a的初值

list<int> a{1,2,3,4,5};
```

## 添加元素

```
push_front(const T& x);                              //头部添加元素
push_back(const T& x);                               //尾部添加元素
insert(iteratpr it, const T& x);                     //任意位置插入一个元素
insert(iterator it, int n, const T& x);              //任意位置插入n个相同元素
insert(iterator it, iterator first, iterator last);  //插入另一个向量的[first,last]间的数据
```

## 删除元素

```
pop_front();                                         //头部删除元素
pop_back();                                          //尾部删除元素
erase(iterator it);                                  //任意位置删除一个元素
erase(iterator first, iterator last);                //删除[first， last]之间的元素
clear();                                             //清空所有元素
```


## list的内存模型：双向循环链表
![image](https://img-blog.csdnimg.cn/20200425170408783.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzM2OTE1MDc4,size_16,color_FFFFFF,t_70#pic_center)
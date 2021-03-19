## 特性
deque是双向开口的连续存储空间（动态地将多个连续空间通过指针数组接合在一起），除了提供较快的随机访问之外（比vector稍慢一点），在头尾两端分别做插入和删除都是常数时间；所以正如词汇含义所示，deque一般用作双向队列。

## 创建deque
几乎同vector：

```cpp
deque<Elem> c;  创建一个空的deque
deque<Elem> c1(c2);  复制deque,复制跟c1一模一样的队列c2
deque<Elem> c(n);          创建一个deque,元素个数为n,且值均为0
deque<Elem> c(n,num);      创建一个deque,元素个数为n,且值均为num
```

## 数据访问
- 下标[]操作，at()函数，提供较快的随机访问
- front()和back()返回第一个和最后一个数据
- begin()和end()返回迭代器

## 插入数据

```
//常数时间
c.push_back(num);  在尾部加入一个数据num
c.push_front(num);  在头部插入一个数据num

//O(n)
c.insert(pos,num);  在该pos位置的数前面插入一个num
c.insert(pos,n,num);  在该pos位置的数前面插入n个num
c.insert(pos,beg,end);  在该pos位置的数前插入在[beg,end)区间的数据
```

## 删除数据

```
//常数时间
c.pop_back();  删除最后一个数据
c.pop_front();  删除头部数据

//O(n)
c.erase(pos);  删除pos位置的数据
c.erase(beg,end);  删除[beg,end)区间的数据
```

## 排序
在deque实现排序，还不如先复制到vector排完序后再复制到deque

## 内存模型：中控器+缓冲区+迭代器
![image](https://img-blog.csdn.net/20180531203242141)

重载了迭代器++和--的操作，这里注意到达两个缓冲区的边界后的跳跃


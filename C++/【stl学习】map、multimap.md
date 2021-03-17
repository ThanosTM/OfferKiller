#### 特征
KV型的自排序容器，底层是一颗红黑树；对于迭代器而言，只能修改实值，不能修改key

#### map的声明

```cpp
#include <map>
map<string, int> personnel;
```

其中默认的排序方式是按key升序排列（less<>()），自定义排序可以仿照《set、multiset》篇，重载自定义类型的比较运算符，或者重写仿函数

#### map插入

```cpp
//1. insert插入pair数据
mapStudent.insert(pair<int, string>(1, "student_one"));  

//2. insert函数插入value_type数据
mapStudent.insert(map<int, string>::value_type (1, "student_one"));

//3. 用数组方式插入数据
mapStudent[1] = "student_one"; 
```

#### map的大小
mapStudents.size()

#### map元素的查找
- count(key)函数，返回key的出现次数，对于map而言只有0和1，缺点是不能定位
- find(key)函数，返回key的迭代器，iterator是一个pair对象

#### map删除元素erase
- 删除某个key
- 删除某个迭代器
- 删除区间

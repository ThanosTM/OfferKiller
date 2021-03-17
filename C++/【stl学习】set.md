## 特征
- K模型容器（存放同一种数据类型）
- set中每个元素的值是唯一的，而multiset可以有重复元素
- set插入数据可以根据元素的值自动排序，==底层是红黑树==

## set的初始化
#### set的模板参数

```cpp
template < class T,                        // 表示set里面存放的数据类型
           class Compare = less<T>,        // 仿函数，可以指定让set按照什么方式进行比较数据
           class Alloc = allocator<T>     // 空间配置器，默认是系统提供的
         >
```

#### set初始化列表
示例：

```cpp
#include <set>

set<int> S1{3, 4, 0, 2, 8};
set<string, greater<int>> S2;
```

## set插入元素

```cpp
pair<iterator,bool> insert (const value_type& val);
```
解释：iterator返回插入位置的迭代器，bool返回是否插入成功（set不允许插入元素相同的值）

## set遍历
- 迭代器/反向迭代器
- for(auto item : S1){}

## set删除元素
- 删除某个位置
```cpp
void erase (iterator position);
```

- 删除某个值
```cpp
size_type erase (const value_type& val);
```

- 删除区间
```cpp
void erase (iterator first, iterator last);
```

## 其他
#### 查找数据
```
iterator   find (const value_type& val)  const;
```
判断是否查到示例：
```cpp
auto it = S1.find(0);
if(it != S1.end()){
    
}
else{
    
}
```

#### 计数
```
size_type    count (const value_type& val)   const;
```
对于set来讲只有0和1，可用于判断是否存在该元素；而multiset可以拥有大于1个的相同元素


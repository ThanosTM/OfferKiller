## 成员函数：

```
=================迭代器========================= 
begin 　　返回指向容器起始位置的迭代器（iterator） 
end 　　   返回指向容器末尾位置的迭代器 
cbegin　   返回指向容器起始位置的常迭代器（const_iterator） 
cend 　　 返回指向容器末尾位置的常迭代器 
=================Capacity================ 
size  　　 返回有效元素个数 
max_size  返回 unordered_map 支持的最大元素个数 
empty        判断是否为空 
=================元素访问================= 
operator[]  　　   访问元素 
at  　　 　　　　访问元素 
=================元素修改================= 
insert  　　插入元素 
erase　　 删除元素 
swap 　　 交换内容 
clear　　   清空内容 
emplace 　构造及插入一个元素 
emplace_hint 按提示构造及插入一个元素 
================操作========================= 
find 　　　　　　通过给定主键查找元素,没找到：返回unordered_map::end
count 　　　　　返回匹配给定主键的元素的个数 
equal_range 　　返回值匹配给定搜索值的元素组成的范围 
================Buckets====================== 
bucket_count 　　　返回槽（Bucket）数 
max_bucket_count    返回最大槽数 
bucket_size 　　　   返回槽大小 
bucket 　　　　　　返回元素所在槽的序号 
load_factor　　　　 返回载入因子，即一个元素槽（Bucket）的最大元素数 
max_load_factor 　  返回或设置最大载入因子 
rehash　　　　　　 设置槽数 
reserve 　　　　　  请求改变容器容量
```

## unordered_map内存模型
底层用开链法解决哈希冲突，用哈希桶实现
![image](https://img-blog.csdn.net/20170220095430216)
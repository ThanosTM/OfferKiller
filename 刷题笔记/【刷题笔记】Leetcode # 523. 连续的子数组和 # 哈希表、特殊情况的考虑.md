## 原题回顾
给定一个包含 非负数 的数组和一个目标 整数 k，编写一个函数来判断该数组是否含有连续的子数组，其大小至少为 2，且总和为 k 的倍数，即总和为 n*k，其中 n 也是一个整数。

 

示例 1：

输入：[23,2,4,6,7], k = 6
输出：True
解释：[2,4] 是一个大小为 2 的子数组，并且和为 6。
示例 2：

输入：[23,2,6,4,7], k = 6
输出：True
解释：[23,2,6,4,7]是大小为 5 的子数组，并且和为 42。
 

说明：

数组的长度不会超过 10,000 。
你可以认为所有数字总和在 32 位有符号整数范围内。

## 实现V1.0

```cpp
bool checkSubarraySum(vector<int>& nums, int k) {
    int left, right;
    for(left = 0; left < nums.size()-1; left++){
        int sum = nums[left];
        for(right = left+1; right < nums.size(); right++){
            sum += nums[right];
            if(!k && !sum)return true;
            else if(!k)continue;
            else if(sum%k == 0)return true;
        }
    }
    return false;
}

```

```
执行用时: 116 ms
内存消耗: 24.8 MB
```

## 学习笔记
##### 特殊情形的考虑
在本题中，需要注意题目条件中的整数k包含+/-/0三种情况，其中0情况比较特殊，不能用于取模运算；当k = 0时，需要单独考虑，如[0,0]、[5,0,0]均为true.

##### 性能分析
实现V1.0的时间复杂度为O(n^2)，空间复杂度为O(1)；有没有更好的算法呢？

##### 基于哈希表
- 哈希表的访问时间具有O(1)的复杂度；
- 这个想法非常巧妙，对于每个元素i，记录到i的元素和sum%k和编号i；考虑这样一个事实：若有两个sum%k是相同的，编号分别为i和j，则[i+1, ..., j]之和必为k的倍数
- 总的复杂度为O(n)

## 实现V2.0

```cpp
bool checkSubarraySum(vector<int>& nums, int k) {
    for(int i = 0; i < nums.size()-1; i++){
        if(nums[i] == 0 && nums[i+1] == 0)return true;
    }
    if(!k)return false;
    if(k < 0)k = -k;
    unordered_map<int, int> map;
    map[0] = -1;
    int sum = 0;
    for(int i = 0; i < nums.size(); i++){
        sum += nums[i];
        if(map.find(sum%k) != map.end()){
            if(i-map[sum%k] > 1)
                return true;
        }
        else
            map[sum%k] = i;
    }
    return false;
}
```

```
执行用时: 72 ms
内存消耗: 25.8 MB
```
分析：执行用时可以预见地大幅缩减了，但内存消耗不减反增，这是因为哈希表本身就是用内存开销换取时间的特点。






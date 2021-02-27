## 原题回顾
给定一个只包含正整数的非空数组。是否可以将这个数组分割成两个子集，使得两个子集的元素和相等。

注意:

每个数组中的元素不会超过 100，数组的大小不会超过 200

```
示例 1:
输入: [1, 5, 11, 5]
输出: true
解释: 数组可以分割成 [1, 5, 5] 和 [11].

示例 2:
输入: [1, 2, 3, 5]
输出: false
解释: 数组不能分割成两个元素和相等的子集.
```

## 先看一个经典的问题
#### 0-1背包问题
有n种物品， 每种只有一个。 第i种物品的体积为Vi， 重量为Wi。 选一些物品装到一个容量为C的背包，使得背包内物品在总体积不超过C的前提下重量尽量大。1≤n≤100， 1≤Vi≤C≤10000， 1≤Wi≤10^6。

#### 动态规划
设d(i, j)为将第i, i+1, ..., n个物品放入容量为j的背包后的总重量，
则状态转移方程为d(i, j) = max{d(i+1, j), d(i+1, j-V[i])+W[i]}

即：第i个背包放或不放

#### 动态规划实现

```cpp
for(int i = vt.size()-1; i >= 0; i--){
    for(int j = 0; j <= C; j++){
        if(j >= vt[i].V)d[i][j] = max(d[i+1][j], d[i+1][j-vt[i].V]+vt[i].W);
        else {
            d[i][j] = d[i+1][j];
        }
        d[i+1][j-vt[i].V]+vt[i].W);
    }
}
```

#### 点评
这种方法的空间复杂度为O(n^2)，注意到每次更新时，其实只用到了上一行的dp值，因此可以进一步将空间复杂度降低为O(n)，（见下例），但需要打印选物品的策略时，还是需要O(n^2)的空间进行记录。

## 回到本题
可以先将数组的和求出来，当和为偶数n时则可能存在等和子集，且每个子集和为n/2。

#### 状态定义
设bool型的d(i, j)表示，使用[0, i]子区间内选出一些数能否使和为j，那么很显然d(0, nums[0]) = true;

#### 状态转移方程
分3类情形：
- 不选择nums[i]，dp[i][j] = dp[i-1][j]
- 选择nums[i]，dp[i][j] = dp[i-1][j - nums[i]]，需要满足j >= nums[i]
- 还有一种特殊情形，nums[i] = j，直接选择nums[i]即可满足j的需求

## 实现V1.0

```cpp
class Solution {
public:
    bool canPartition(vector<int>& nums) {
        int sum = 0, maxn = 0;
        for(int i = 0; i < nums.size(); i++){
            sum += nums[i];
            maxn = max(maxn, nums[i]);
        }
        int target = sum/2;
        if(sum&1 || maxn > target)return false;
        vector<vector<int>> dp(nums.size()+1, vector<int>(target+1, 0));
        for(int i = nums.size()-1; i >= 0; i--){
            for(int j = 0; j <= target; j++){
                dp[i][j] = dp[i+1][j] || j==nums[i];
                if(j >= nums[i])dp[i][j] = dp[i][j] || dp[i+1][j - nums[i]];
            }
        }
        return dp[0][target];
    }
};
```

```
执行用时: 396 ms
内存消耗: 74.3 MB
```

点评：可以使用一维滚动数组dp来记录，此时需要注意两个循环的方向

## 实现V2.0

```cpp

class Solution {
public:
    bool canPartition(vector<int>& nums) {
        int sum = 0, maxn = 0;
        for(int i = 0; i < nums.size(); i++){
            sum += nums[i];
            maxn = max(maxn, nums[i]);
        }
        int target = sum/2;
        if(sum&1 || maxn > target)return false;
        vector<int> dp(target+1, 0);
        dp[0] = 1;
        for(int i = 0; i < nums.size(); i++){
            int num = nums[i];
            for(int j = target; j >= num; j--){
                dp[j] |= dp[j - num];
            }
        }
        return dp[target];
    }
};
```

点评：想一想什么情况下，两个循环方向会出现问题，导致结果出现错误？
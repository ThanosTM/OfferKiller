## 原题回顾
给定一个包含 n 个整数的数组 nums 和一个目标值 target，判断 nums 中是否存在四个元素 a，b，c 和 d ，使得 a + b + c + d 的值与 target 相等？找出所有满足条件且不重复的四元组。

注意：

答案中不可以包含重复的四元组。

```
示例：

给定数组 nums = [1, 0, -1, 0, -2, 2]，和 target = 0。

满足要求的四元组集合为：
[
  [-1,  0, 0, 1],
  [-2, -1, 1, 2],
  [-2,  0, 0, 2]
]
```

## 先看个三数之和
#### 题目描述
给你一个包含 n 个整数的数组 nums，判断 nums 中是否存在三个元素 a，b，c ，使得 a + b + c = 0 ？请你找出所有满足条件且不重复的三元组。

注意：答案中不可以包含重复的三元组。

```
示例：

给定数组 nums = [-1, 0, 1, 2, -1, -4]，

满足要求的三元组集合为：
[
  [-1, 0, 1],
  [-1, -1, 2]
]
```

#### 思路
- 先对其进行排序；
- 对每一个nums[i]，取双指针指向i+1和size()-1，根据3数之和与target的大小关系调整left和right指针，若相等即找到一解；
- 对于重复元素的去除
- 剪枝，对提前可以判断不存在的i退出循环
- 时间复杂度分析：O(n^2)

#### 实现代码

```cpp
class Solution {
public:
    vector<vector<int>> threeSum(vector<int>& nums) {
        vector<vector<int>> ans;
        sort(nums.begin(), nums.end());
        for(int i = 0; i < nums.size(); i++){
            if(nums[i] > 0)break;
            if(i > 0 && nums[i] == nums[i-1])continue;
            int left = i+1, right = nums.size()-1;
            while(left < right){
                if(nums[i] + nums[left] + nums[right] == 0){
                    ans.emplace_back(vector<int>{nums[i], nums[left], nums[right]});
                    int templ = nums[left], tempr = nums[right];
                    while(nums[++left] == templ && left < right);
                    while(nums[--right] == tempr && left < right);
                }
                else if(nums[i] + nums[left] + nums[right] < 0){
                    left++;
                }
                else {
                    right--;
                }
            }
        }
        return ans;
    }
};
```

#### 一个衍生版本
给定一个包括 n 个整数的数组 nums 和 一个目标值 target。找出 nums 中的三个整数，使得它们的和与 target 最接近。返回这三个数的和。假定每组输入只存在唯一答案。

 

示例：

输入：nums = [-1,2,1,-4], target = 1
输出：2
解释：与 target 最接近的和是 2 (-1 + 2 + 1 = 2) 。

同样的思路，甚至还要更简单一些；

#### 实现代码

```cpp
class Solution {
public:
    int threeSumClosest(vector<int>& nums, int target) {
        int ans = nums[0] + nums[1] + nums[2];
        sort(nums.begin(), nums.end());
        for(int i = 0; i < nums.size(); i++){
            int left = i+1, right = nums.size()-1;
            while(left < right){
                int sum = nums[i] + nums[left] + nums[right];
                if(abs(sum-target) < abs(ans-target))ans = sum;
                if(sum < target){
                    left++;
                }
                else if(sum > target){
                    right--;
                }
                else {
                    return target;
                }
            }
        }
        return ans;
    }
};
```

## 回到本题
同样是，排序，针对每个i, j利用双指针，只不过增加了一层循环，时间复杂度来到O(n^3)；

```cpp
class Solution {
public:
    vector<vector<int>> fourSum(vector<int>& nums, int target) {
        vector<vector<int>> ans;
        if(nums.size() < 4)return ans;
        sort(nums.begin(), nums.end());
        int length = nums.size();
        for(int i = 0; i < length-3; i++){
            if (nums[i] + nums[i + 1] + nums[i + 2] + nums[i + 3] > target) {
                break;
            }
            if (nums[i] + nums[length - 3] + nums[length - 2] + nums[length - 1] < target) {
                continue;
            }
            if(i > 0 && nums[i] == nums[i-1])continue;
            for(int j = i+1; j < length-2; j++){
                if(j > i+1 && nums[j] == nums[j-1])continue;
                if (nums[i] + nums[j] + nums[j + 1] + nums[j + 2] > target) {
                    break;
                }
                if (nums[i] + nums[j] + nums[length - 2] + nums[length - 1] < target) {
                    continue;
                }
                int left = j+1, right = length-1;
                while(left < right){
                    if(nums[i] + nums[j] + nums[left] + nums[right] == target){
                        ans.emplace_back(vector<int>{nums[i], nums[j], nums[left], nums[right]});
                        int templ = nums[left], tempr = nums[right];
                        while(templ == nums[left] && left < right)left++;
                        while(tempr == nums[right] && left < right)right--;
                    }
                    else if(nums[i] + nums[j] + nums[left] + nums[right] > target){
                        right--;
                    }
                    else{
                        left++;
                    }
                }
            }
        }
        return ans;
    }
};
```





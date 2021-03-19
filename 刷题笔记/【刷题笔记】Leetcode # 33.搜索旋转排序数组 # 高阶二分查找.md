## 原题回顾
整数数组 nums 按升序排列，数组中的值 互不相同 。

在传递给函数之前，nums 在预先未知的某个下标 k（0 <= k < nums.length）上进行了 旋转，使数组变为 [nums[k], nums[k+1], ..., nums[n-1], nums[0], nums[1], ..., nums[k-1]]（下标 从 0 开始 计数）。例如， [0,1,2,4,5,6,7] 在下标 3 处经旋转后可能变为 [4,5,6,7,0,1,2] 。

给你 旋转后 的数组 nums 和一个整数 target ，如果 nums 中存在这个目标值 target ，则返回它的索引，否则返回 -1 。

```
示例 1：

输入：nums = [4,5,6,7,0,1,2], target = 0
输出：4
示例 2：

输入：nums = [4,5,6,7,0,1,2], target = 3
输出：-1
示例 3：

输入：nums = [1], target = 0
输出：-1
 

提示：

1 <= nums.length <= 5000
-10^4 <= nums[i] <= 10^4
nums 中的每个值都 独一无二
nums 肯定会在某个点上旋转
-10^4 <= target <= 10^4
```

## 先看两个简单的例子

#### 153. 寻找旋转排序数组中的最小值
此题不含有重复元素

```cpp
class Solution {
public:
    int findMin(vector<int>& nums) {
        int left = 0;
        int right = nums.size() - 1;
        while (left < right) {
            int mid = left + (right - left) / 2;
            if (nums[mid] > nums[right]) {          
                left = mid + 1;
            } 
            else if(nums[mid] < nums[right]){                               
                right = mid;
            }
            else if(nums[mid] == nums[right]){
                right = mid;
            }
        }
        return nums[left];
    }
};
```

#### 154.寻找旋转排序数组中的最小值 II
此题可能含有相同元素，同时也是《剑指offer》上的例题，此题最关键的就是要区分一下两种情况：

```
1 1 1 0 1
^   ^   ^

1 0 1 1 1
^   ^   ^
```

这两种情况均根据left、right、mid判断不出下一个区间，因此书上采用遍历该区间的方式：

```
class Solution {
public:
    int findMin(vector<int>& nums) {
        int left = 0;
        int right = nums.size() - 1;
        while (left < right) {
            int mid = left + (right - left) / 2;
            if (nums[mid] > nums[right]) {          
                left = mid + 1;
            } 
            else if(nums[mid] < nums[right]){               
                right = mid;
            }
            else if(nums[mid] == nums[right]){
                int minv = nums[left];
                for(int i = left; i <= right; i++){
                    minv = min(minv, nums[i]);
                }
                return minv;
            }
        }
        return nums[left];
    }
};
```

这里再提供一种方式，对于大多数数组的性能更好，因为不是每一次都需要扫描整个区间。

```
class Solution {
public:
    int findMin(vector<int>& nums) {
        int left = 0;
        int right = nums.size() - 1;
        while (left < right) {
            int mid = left + (right - left) / 2;
            if (nums[mid] > nums[right]) {          
                left = mid + 1;
            } 
            else if(nums[mid] < nums[right]){                       
                right = mid;
            }
            else if(nums[mid] == nums[right]){
                right--;
            }
        }
        return nums[left];
    }
};
```


## 回到这题
一种思路是先获得最小值及其位置，再进行常规的二分查找

```
class Solution {
public:
    int search(vector<int>& nums, int target) {
        int vsize = nums.size();
        int left = 0, right = vsize-1;
        while(left < right){
            int mid = left + (right-left)/2;
            if(nums[mid] > nums[right]){
                left = mid + 1;
            }
            else if(nums[mid] < nums[right]){
                right = mid;
            }
            else if(nums[mid] == nums[right]){
                right = mid;
            }
        }

        if(target == nums[vsize-1]){
            return vsize-1;
        }
        else if(target > nums[vsize-1]){
            right = left-1, left = 0;
        }
        else{
            right = vsize-1;
        }

        while(left <= right){
            int mid = left + (right-left)/2;
            if(nums[mid] == target){
                return mid;
            }
            else if(nums[mid] < target){
                left = mid + 1;
            }
            else if(nums[mid] > target){
                right = mid - 1;
            }
        }

        return -1;
    } 
};
```

有一种更高效的查找方式，但逻辑较为复杂，需要仔细考虑清楚：

```cpp
class Solution {
public:
    int search(vector<int>& nums, int target) {
        int vsize = nums.size();
        int left = 0, right = vsize-1;
        while(left <= right){
            int mid = left + (right-left)/2;
            if(nums[mid] == target) return mid;

            if(nums[mid] >= nums[0]){
                if(target >= nums[left] && target < nums[mid]){
                    right = mid - 1;
                }
                else {
                    left = mid + 1;
                }
            }
            else{
                if(target > nums[mid] && target <= nums[vsize-1]){
                    left = mid + 1;
                }
                else{
                    right = mid - 1;
                }
            }
        }
        return -1;
    } 
};
```

## 若存在相同元素情形下的搜索呢？——81. 搜索旋转排序数组 II



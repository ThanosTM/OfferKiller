## 原题回顾

给你两个数组，arr1 和 arr2，

arr2 中的元素各不相同
arr2 中的每个元素都出现在 arr1 中
对 arr1 中的元素进行排序，使 arr1 中项的相对顺序和 arr2 中的相对顺序相同。未在 arr2 中出现过的元素需要按照升序放在 arr1 的末尾。

 

示例：

输入：arr1 = [2,3,1,3,2,4,6,7,9,2,19], arr2 = [2,1,4,3,9,6]

输出：[2,2,2,1,4,3,3,9,6,7,19]

## 我的解答

```cpp
class Solution {
public:
    vector<int> relativeSortArray(vector<int>& arr1, vector<int>& arr2) {
        sort(arr1.begin(), arr1.end());
        vector<int> ans;
        int size2 = arr2.size();
        for(int i = 0; i < size2; i++){
            auto itl = lower_bound(arr1.begin(), arr1.end(), arr2[i]);
            auto itu = upper_bound(arr1.begin(), arr1.end(), arr2[i]);
            ans.insert(ans.end(), itl, itu);
            arr1.erase(itl, itu);
        }
        ans.insert(ans.end(), arr1.begin(), arr1.end());
        return ans;
    }
};
```

## 拓宽思路：还能这么玩

```cpp
class Solution {
public:
    vector<int> relativeSortArray(vector<int>& arr1, vector<int>& arr2) {
        unordered_map<int, int> rank;
        for (int i = 0; i < arr2.size(); ++i) {
            rank[arr2[i]] = i;
        }
        sort(arr1.begin(), arr1.end(), [&](int x, int y) {
            if (rank.count(x)) {
                return rank.count(y) ? rank[x] < rank[y] : true;
            }
            else {
                return rank.count(y) ? false : x < y;
            }
        });
        return arr1;
    }
};

```


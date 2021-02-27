## 原题回顾

给定一个二叉树和一个目标和，找到所有从根节点到叶子节点路径总和等于给定目标和的路径。

说明: 叶子节点是指没有子节点的节点。

示例:
给定如下二叉树，以及目标和 sum = 22，

```
              5
             / \
            4   8
           /   / \
          11  13  4
         /  \    / \
        7    2  5   1
```

返回:


```
[
   [5,4,11,2],
   [5,8,4,5]
]
```

## 实现V1.0


```cpp
class Solution {
public:
    vector<vector<int>> pathSum(TreeNode* root, int sum) {
        if(!root)return vector<vector<int>>();
        else if(!root->left && !root->right && sum == root->val)
            return vector<vector<int>>{vector<int>{root->val}};
        vector<vector<int>> vresult, vleft, vright;
        vleft = pathSum(root->left, sum-root->val);
        vright = pathSum(root->right, sum-root->val);
        for(auto it:vleft){
            vresult.push_back(vector<int>{root->val});
            vresult[vresult.size()-1].insert(vresult[vresult.size()-1].end(), it.begin(), it.end());
        }
        for(auto it:vright){
            vresult.push_back(vector<int>{root->val});
            vresult[vresult.size()-1].insert(vresult[vresult.size()-1].end(), it.begin(), it.end());
        }
        return vresult;
    }
    vector<vector<int>> res;
    vector<int> path;

    void dfs(TreeNode *root, int sum){
        if(!root){
            //path.clear();
            return;
        }
        else if(!root->left && !root->right && sum == root->val){
            path.push_back(root->val);
            res.push_back(path);
            path.pop_back();
            return;
        }
        path.push_back(root->val);
        dfs(root->left, sum-root->val);
        dfs(root->right, sum-root->val);
        path.pop_back();
    }

    vector<vector<int>> pathSum(TreeNode* root, int sum) {
        dfs(root, sum);
        return res;
    }
};

```

```
执行用时: 52 ms
内存消耗: 26.9 MB
```

## 学习笔记
##### 性能分析
首次提交在执行速度上仅超过8%的用户，可见性能非常差；具体分析原因是因为涉及到大量的vector构造、插入、复制等操作，并且在迭代过程中，将产生大量的局部变量栈开销；

##### 改进
==可以考虑采用全局变量代替局部变量==，方便操作又降低了开销

##### DFS&回溯法
之前接触到过的DFS均是遍历一遍就结束，对于本题，达到路径终点后还需要退回到上一个节点，同时路径记录也要退回一格，这就是回溯法；

```cpp
path.push_back(root->val);
if(!root->left && !root->right && sum == root->val){        
    res.push_back(path);
}
dfs(root->left, sum-root->val);
dfs(root->right, sum-root->val);
path.pop_back();
```
其中，关键在于最后一句，将路径记录退回一格，继续进行试探。

## 实现V2.0

```
class Solution {
public:
    vector<vector<int>> res;
    vector<int> path;
    
    void dfs(TreeNode *root, int sum){
        if(!root){
            return;
        }
        path.push_back(root->val);
        if(!root->left && !root->right && sum == root->val){        
            res.push_back(path);
        }
        dfs(root->left, sum-root->val);
        dfs(root->right, sum-root->val);
        path.pop_back();
    }

    vector<vector<int>> pathSum(TreeNode* root, int sum) {
        dfs(root, sum);
        return res;
    }
};
```

```
执行用时: 8 ms
内存消耗: 16.7 MB
```










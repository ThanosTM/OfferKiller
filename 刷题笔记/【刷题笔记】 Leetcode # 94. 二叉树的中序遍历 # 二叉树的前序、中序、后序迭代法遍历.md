## 原题回顾
给定一个二叉树，返回它的中序 遍历。

示例:
```
输入: [1,null,2,3]
   1
    \
     2
    /
   3
```

输出: [1,3,2]
进阶: 递归算法很简单，你可以通过迭代算法完成吗？

## 前序、中序、后序迭代法整理
#### 前序

```cpp
class Solution {
public:
    vector<int> preorderTraversal(TreeNode* root) {
        vector<int> ans;
        if(root == nullptr)return ans;
        stack<TreeNode *> st;
        st.push(root);

        while(!st.empty()){
            TreeNode *top = st.top();
            st.pop();
            ans.emplace_back(top->val);
            
            if(top->right)st.push(top->right);
            if(top->left)st.push(top->left);
        }
        
        return ans;
    }
};
```


#### 中序：尽可能向左

```cpp
class Solution {
public:
    vector<int> inorderTraversal(TreeNode* root) {
        vector<int> ans;
        stack<TreeNode *> st;
        TreeNode *p = root;
        while(p || !st.empty()){
            while(p){
                st.push(p);
                p = p->left;
            }
            p = st.top();
            st.pop();
            ans.emplace_back(p->val);
            p = p->right;
        }
        return ans;
    }
};
```

#### 后序：用一个cur指针标记当前退出的节点

```cpp
class Solution {
public:
    vector<int> postorderTraversal(TreeNode* root) {
        vector<int> ans;
        if(root == nullptr)return ans;

        stack<TreeNode *> stk;
        stk.push(root);
        TreeNode *cur = root;

        while(!stk.empty()){
            TreeNode *top = stk.top();
            if(top->left != nullptr && cur != top->left && cur != top->right){
                stk.push(top->left);
            }
            else if(top->right != nullptr && cur != top->right){
                stk.push(top->right);
            }
            else {
                stk.pop();
                ans.emplace_back(top->val);
                cur = top;
            }
        }

        return ans;
    }
};
```

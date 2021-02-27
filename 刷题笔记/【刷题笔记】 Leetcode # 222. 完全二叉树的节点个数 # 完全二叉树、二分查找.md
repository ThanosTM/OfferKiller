## 原题回顾
给出一个完全二叉树，求出该树的节点个数。

说明：

完全二叉树的定义如下：在完全二叉树中，除了最底层节点可能没填满外，其余每层节点数都达到最大值，并且最下面一层的节点都集中在该层最左边的若干位置。若最底层为第 h 层，则该层包含 1~ 2h 个节点。

示例:

```
输入: 
    1
   / \
  2   3
 / \  /
4  5 6
```

输出: 6

## 最暴力的解法：遍历计数

```cpp
class Solution {
public:
    int count = 0;
    void traverse(TreeNode *root){
        if(!root)return;
        count++;
        traverse(root->left);
        traverse(root->right);
    }
    int countNodes(TreeNode* root) {
        traverse(root);
        return count;
    }
}
```

需要遍历所有节点，时间复杂度为O(n)

## 二分查找...一开始没有想到

```cpp
class Solution {
public:
    int countNodes(TreeNode* root) {
        if(!root)return 0;
        TreeNode *node = root;
        int level = 0;
        while(node){
            node = node->left;
            level++;
        }

        int left = (1 << (level-1)), right = (1 << level) - 1;
        while(left < right){
            int mid = (left + right) >> 1;
            bool exist = isNodeExist(root, level, mid);
            if(exist){
                left = mid + 1;
            }
            else {
                right = mid - 1;
            }
        }

        if(isNodeExist(root, level, left))return left;
        else return left-1;
    }

    bool isNodeExist(TreeNode *root, int level, int offset){
        TreeNode *node = root;
        level--;
        while(node && level>0){
            int dir = (offset >> (--level)) & 0x1;
            if(dir)node = node->right;
            else node = node->left;
        }
        return node!=nullptr;
    }
};
```

时间复杂度分析：对最底层进行二分查找O(logn)，每次二分查找需要判断该节点是否存在O(logn)，总的时间复杂度为O((logn)^2)

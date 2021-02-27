## 原题回顾
给定一个链表，删除链表的倒数第 n 个节点，并且返回链表的头结点。

示例：

给定一个链表: 1->2->3->4->5, 和 n = 2.

当删除了倒数第二个节点后，链表变为 1->2->3->5.
说明：

给定的 n 保证是有效的。

进阶：

你能尝试使用一趟扫描实现吗？

## 题目分析
该题常规方法多趟扫描实现很简单，而使用双指针法能够实现仅用一趟扫描就可完成节点的删除

## 自己的实现

```cpp
class Solution {
public:
    ListNode* removeNthFromEnd(ListNode* head, int n) {
        ListNode *right = head;
        ListNode *prehead = new ListNode(0);
        prehead->next = head;
        ListNode *left = prehead;
        int count = 0;
        while(n--){
            right = right->next;
        }
        while(right){
            right = right->next;
            left = left->next;
        }
        ListNode *del = left->next;
        left->next = del->next;
        delete del;
        ListNode *ret = prehead->next;
        delete prehead;
        return ret;
    }
};
```

## 一些讨论
- 这里由题意假定n始终是有效的，若没有这个条件，还需要考虑当n大于整个链表的长度的情况；
- 同样地，使用一个哑结点可以将删除第一个节点的操作与其余的相统一；
- 在C++中，被删除的节点和哑结点均应该再返回前delete掉（官方题解使用Java不需要释放内存）



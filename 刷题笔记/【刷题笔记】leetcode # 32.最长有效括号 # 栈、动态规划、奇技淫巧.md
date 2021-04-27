## 原题回顾
给你一个只包含 '(' 和 ')' 的字符串，找出最长有效（格式正确且连续）括号子串的长度。

```
示例 1：

输入：s = "(()"
输出：2
解释：最长有效括号子串是 "()"
示例 2：

输入：s = ")()())"
输出：4
解释：最长有效括号子串是 "()()"
示例 3：

输入：s = ""
输出：0
```


## 我的解法
一般遇到括号匹配的题目直接能想到的是使用栈解决，考虑两次扫描该字符串，第一次扫描利用栈将匹配的字符做出标记，第二次扫描检测最长的连续标记。时间复杂度O(N)，空间复杂度O(N).
```cpp
int longestValidParentheses(string s) {
    int result = 0;
    int slen = s.length();
    vector<bool> isvalid(slen, false); 
    stack<pair<char, int>> stk;
    for(int i = 0; i < slen; i++){
        if(s[i] == '('){
            stk.push({'(', i});
        }
        else if(s[i] == ')'){
            if(!stk.empty()){
                pair<char, int> temp = stk.top();
                stk.pop();
                isvalid[temp.second] = true;
                isvalid[i] = true;
            }
        }
    }
    int tempmax = 0;
    for(int i = 0; i < slen; i++){
        if(isvalid[i]){
            tempmax++;
            result = max(result, tempmax);
        }
        else{
            tempmax = 0;
        }
    }
    return result;
}
```

## 动态规划
将dp[i]定义为以i为结尾的最长的长度，考虑连续的两个字符s[i]和s[i-1]
1. 若s[i] == ')'，s[i-1] == '('，则dp[i] = dp[i-2] + 2
2. 若s[i] == ')'，s[i-1] == ')'，同时需要考虑dp[i-dp[i-1]-1]是否为'('，则dp[i] = dp[i-1] + dp[i-dp[i-1]-2] + 2
```
s = "( ) ( ( ( ) ) ) )"
dp   0 2 0 0 0 2 4 8 0
```

```cpp
int longest_packet2(string s){
    int result = 0;
    int slen = s.length();
    vector<int> dp(slen, 0);

    for(int i = 1; i < slen; i++){
        if(s[i] == ')' && s[i-1] == '('){
            dp[i] += 2;
            if(i - 2 >= 0) dp[i] += dp[i-2];
        }
        else if(s[i] == ')' && s[i-1] == ')'){
            int index = i - dp[i-1] - 1;
            if(index >= 0 && s[index] == '(') {
                dp[i] += 2;
                dp[i] += dp[i-1];
                if(index-1 >= 0) dp[i] += dp[index-1];
            }
        }
        result = max(result, dp[i]);
    }
    return result;
}
```
时间复杂度O(N)，空间复杂度O(N)

## 左右扫描相结合（不需要额外空间）
使用两个计数器left和right，遇到'('则left++，反之则right++，当从左向右扫描时有：当left == right时说明左右括号已经匹配，此时记录当前最长长度，当left < right则计数器清零；但这种方法不能处理形如“()((())”的字符串，解决方法是再从右向左扫描一遍。
```cpp
int longest_packet3(string s){
    int left, right;
    int slen = s.length();
    int result = 0;

    left = right = 0;
    for(int i = 0; i < slen; i++){
        if(s[i] == '(') left++;
        else right++;

        if(left == right) result = max(result, left * 2);
        else if(left < right) left = right = 0;
    }

    left = right = 0;
    for(int i = slen-1; i >= 0; i--){
        if(s[i] == '(') left++;
        else right++;

        if(left == right) result = max(result, left * 2);
        else if(left > right) left = right = 0;
    }

    return result;
}
```
时间复杂度O(N)，空间复杂度O(1).


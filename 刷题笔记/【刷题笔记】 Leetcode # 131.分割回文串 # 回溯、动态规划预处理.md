## 原题回顾
给定一个字符串 s，将 s 分割成一些子串，使每个子串都是回文串。

返回 s 所有可能的分割方案。

示例:

```
输入: "aab"
输出:
[
  ["aa","b"],
  ["a","a","b"]
]
```

## 经典回溯算法（自己写出来的）

```cpp
class Solution {
public:
    vector<vector<string>> ans;
    vector<vector<string>> partition(string s) {
        vector<string> part;
        backtracking(s, 0, part);
        return ans;
    }

    void backtracking(string &s, int pos, vector<string> &part){
        if(pos >= s.length()){
            ans.emplace_back(part);
            return;
        }
        for(int i = pos; i < s.length(); i++){
            if(isRight(s, pos, i)){
                part.emplace_back(s.substr(pos, i-pos+1));
                backtracking(s, i+1, part);
                part.pop_back();
            }
        }
    }

    bool isRight(string &s, int start, int end){
        while(start <= end){
            if(s[start] != s[end]) return false;
            start++;
            end--;
        }
        return true;
    }
};
```

## 优化：动态规划预处理
经过分析不难发现，每次判断是否为回文串时均要反复调用isRight()函数验证，这可以在回溯算法开始之前先进行预处理，给出dp[i][j]代表字符串s从i到j是否构成回文串，这是一种常用的手段

```cpp
class Solution {
public:
    vector<vector<string>> ans;
    vector<vector<string>> partition(string s) {
        vector<string> part;

        int len = s.length();
        vector<vector<bool>> dp(len, vector<bool>(len, false));
        for(int i = len-1; i >= 0; i--){
            for(int j = i; j < len; j++){
                if(s[i] == s[j] && (j-i <= 2 || dp[i+1][j-1])){
                    dp[i][j] = true;
                }
            }
        }

        backtracking(s, 0, dp, part);
        return ans;
    }

    void backtracking(string &s, int pos, vector<vector<bool>> &dp, vector<string> &part){
        if(pos >= s.length()){
            ans.emplace_back(part);
            return;
        }
        for(int i = pos; i < s.length(); i++){
            if(dp[pos][i]){
                part.emplace_back(s.substr(pos, i-pos+1));
                backtracking(s, i+1, dp, part);
                part.pop_back();
            }
        }
    }

    bool isRight(string &s, int start, int end){
        while(start <= end){
            if(s[start] != s[end]) return false;
            start++;
            end--;
        }
        return true;
    }
};
```



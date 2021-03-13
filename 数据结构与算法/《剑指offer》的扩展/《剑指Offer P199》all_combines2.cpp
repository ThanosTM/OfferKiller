/**
 * @brief：【剑指offer P199】输入一组字母，输出字符的所有组合——二进制法
 *          缺点是，适用的字母位数有限
 * @example：输入"abc"，打印"a", "b", "c", "ab", "ac", "bc", "abc"
 * @author：seu_zhouyi
 */

#include <iostream>
#include <vector>
#include <cassert>
using namespace std;

void combines();

string S = "abcd";
vector<string> list;

int main(){
    combines();
    for(auto item : list){
        cout << item << endl;
    }
    return 0;
}

/**
 * @brief：例如一共有4个字母，mask从0001加到1111,
 */
void combines(){
    int S_len = S.length();
    assert(S_len > 0);

    unsigned int max = (1 << S_len);
    for(int i = 1; i < max; i++){
        int mask = (max >> 1);
        string sub;
        for(int j = 0; j < S_len; j++){
            if((mask >> j) & i){
                sub += S[j];
            }
        }
        list.push_back(sub);
    }
}
#include <algorithm>
#include <iostream>
#include <iterator>
#include <random>
#include <vector>

using namespace std;

int main()
{
    // 用系统的随机数初始化 MT19937 伪随机数引擎
    mt19937 engine{random_device{}()};
    // 使用均匀整数分布
    uniform_int_distribution dist{1, 1000};
    vector<int> v;
    v.reserve(100);
    for (int i = 0; i < 100; ++i) {
        // 产生随机数
        v.push_back(dist(engine));
    }
    copy(v.begin(), v.end(), ostream_iterator<int>(cout, " "));
    cout << '\n';
}

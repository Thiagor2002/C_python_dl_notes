#include <algorithm>       // std::sort
#include <functional>      // std::less
#include <iostream>        // std::cout/endl
#include <vector>          // std::vector
#include "output_range.h"  // operator<< for containers

using namespace std;

int main()
{
    vector v1{1, 7, 3, 2, 9, 4};
    vector v2 = v1;
    auto less_obj = [](auto&& x, auto&& y) {
        return forward<decltype(x)>(x) < forward<decltype(y)>(y);
    };
    sort(v1.begin(), v1.end(), less<>{});
    sort(v2.begin(), v2.end(), less_obj);
    cout << v1 << endl;
    cout << v2 << endl;
}

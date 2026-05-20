#include <array>        // std::array
#include <iostream>     // std::cout
#include <string_view>  // std::string_view
#include <utility>      // std::pair

using namespace std;

int main()
{
    constexpr string_view sv{"hi"};
    constexpr pair pr{sv[0], sv[1]};
    constexpr array a{pr.first, pr.second};
    constexpr int n1 = a[0];
    constexpr int n2 = a[1];
    cout << n1 << ' ' << n2 << '\n';
}

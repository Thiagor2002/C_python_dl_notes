#include <algorithm>  // std::ranges::copy
#include <iostream>   // std::cout/endl
#include <iterator>   // std::ostream_iterator
#include <ranges>     // std::ranges::views

int main()
{
    namespace views = std::ranges::views;
    int a[] = {1, 7, 3, 6, 5, 2, 4, 8};
    std::ranges::copy(a, std::ostream_iterator<int>(std::cout, " "));
    std::cout << std::endl;
    auto r = a | views::filter([](int i) { return i % 2 == 0; })
               | views::reverse;
    std::ranges::copy(r, std::ostream_iterator<int>(std::cout, " "));
    std::cout << std::endl;
}

#include <array>     // std::array
#include <iostream>  // std::cout/endl
#include <vector>    // std::vector
#include <gsl/span>  // gsl::span

void analyse_sequence(gsl::span<int> sp)
{
    std::cout << "Analysing span of size " << sp.size() << std::endl;
    for (auto& item : sp) {
        std::cout << item << ' ';
    }
    std::cout << std::endl;
}

int main()
{
    {
        int a[] = {1, 2, 3, 4, 5};
        analyse_sequence(a);
    }
    {
        std::array<int, 5> a{1, 2, 3, 4, 5};
        analyse_sequence(a);
    }
    {
        std::vector<int> v{1, 2, 3, 4, 5};
        analyse_sequence(v);
    }
}

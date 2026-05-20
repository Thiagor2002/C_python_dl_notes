#include <stdexcept>  // std::invalid_argument
#include <assert.h>   // assert
#include <stdio.h>    // printf

constexpr int factorial(int n)
{
    //assert(n >= 0);
    if (n < 0) {
        throw std::invalid_argument("Arg must be non-negative");
    }

    if (n == 0) {
        return 1;
    } else {
        return n * factorial(n - 1);
    }
}

int main()
{
    constexpr int n = factorial(10);
    printf("%d\n", n);

    constexpr int bad = factorial(-1);
}

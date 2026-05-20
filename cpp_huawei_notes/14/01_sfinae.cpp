#include <stdio.h>  // puts

struct Test {
    typedef int foo;
};

template <typename T>
void f(typename T::foo)
{
    puts("1");
}

template <typename T>
void f(T)
{
    puts("2");
}

int main()
{
    f<Test>(10);
    f<int>(10);
}

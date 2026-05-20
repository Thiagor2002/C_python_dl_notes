#include <atomic>           // std::atomic
#include <iostream>         // std::cout
#include "scoped_thread.h"  // scoped_thread

#define LOOPS 100000

std::atomic<int> a;
volatile int v;

void increment_atomic(std::atomic<int>& a)
{
    for (int i = 0; i < LOOPS; ++i) {
        ++a;
    }
}

void increment_volatile(volatile int& n)
{
    for (int i = 0; i < LOOPS; ++i) {
        ++n;
    }
}

int main()
{
    {
        std::cout << "*** Incrementing on the same atomic int\n";
        {
            scoped_thread t1{increment_atomic, std::ref(a)};
            scoped_thread t2{increment_atomic, std::ref(a)};
        }
        std::cout << "Result is " << a << std::endl;
    }
    {
        std::cout << "*** Incrementing on the same volatile int\n";
        {
            scoped_thread t1{increment_volatile, std::ref(v)};
            scoped_thread t2{increment_volatile, std::ref(v)};
        }
        std::cout << "Result is " << v << std::endl;
    }
}

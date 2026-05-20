#include <atomic>           // std::atomic
#include <functional>       // std::ref
#include <iostream>         // std::cout
#include <mutex>            // std::mutex
#include "rdtsc.h"          // rdtsc
#include "scoped_thread.h"  // scoped_thread

#define LOOPS 10000

std::atomic<int> a1;
std::atomic<int> a2;
volatile int v1;
volatile int v2;
int i1;
int i2;
std::mutex inc_lock;
std::mutex output_lock;

void increment_atomic(std::atomic<int>& a)
{
    auto t1 = rdtsc();
    for (int i = 0; i < LOOPS; ++i) {
        ++a;
    }
    auto t2 = rdtsc();
    std::lock_guard guard{output_lock};
    std::cout << "increment_atomic: " << (t2 - t1) / LOOPS
              << " cycles on average\n";
}

void increment_volatile(volatile int& n)
{
    auto t1 = rdtsc();
    for (int i = 0; i < LOOPS; ++i) {
        ++n;
    }
    auto t2 = rdtsc();
    std::lock_guard guard{output_lock};
    std::cout << "increment_volatile: " << (t2 - t1) / LOOPS
              << " cycles on average\n";
}

void increment_with_lock(int& n)
{
    auto t1 = rdtsc();
    for (int i = 0; i < LOOPS; ++i) {
        std::lock_guard guard{inc_lock};
        ++n;
    }
    auto t2 = rdtsc();
    std::lock_guard guard{output_lock};
    std::cout << "increment_with_lock: " << (t2 - t1) / LOOPS
              << " cycles on average\n";
}

int main()
{
    {
        std::cout << "*** Testing with one increment_atomic thread\n";
        scoped_thread t{increment_atomic, std::ref(a1)};
    }
    {
        std::cout << "*** Testing with two increment_atomic threads\n";
        scoped_thread t1{increment_atomic, std::ref(a1)};
        scoped_thread t2{increment_atomic, std::ref(a2)};
    }
    {
        std::cout << "*** Testing with two increment_volatile threads\n";
        scoped_thread t1{increment_volatile, std::ref(v1)};
        scoped_thread t2{increment_volatile, std::ref(v2)};
    }
    {
        std::cout << "*** Testing with one increment_volatile thread and "
                     "one increment_atomic thread\n";
        scoped_thread t1{increment_volatile, std::ref(v1)};
        scoped_thread t2{increment_atomic, std::ref(a2)};
    }
    {
        std::cout << "*** Testing with one increment_with_lock thread\n";
        scoped_thread t{increment_with_lock, std::ref(i1)};
    }
    {
        std::cout << "*** Testing with two increment_with_lock threads\n";
        scoped_thread t1{increment_with_lock, std::ref(i1)};
        scoped_thread t2{increment_with_lock, std::ref(i2)};
    }
}

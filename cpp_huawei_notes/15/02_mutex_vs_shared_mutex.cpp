#include <iostream>         // std::cout
#include <mutex>            // std::mutex
#include <random>           // std::mt19937/uniform_int_distribution
#include <shared_mutex>     // std::shared_mutex
#include <vector>           // std::vector
#include "rdtsc.h"          // rdtsc
#include "scoped_thread.h"  // scoped_thread

using namespace std;

mutex mtx;
shared_mutex shared_mtx;

int counter = 0;

#define LOOPS 10000
#define THRDS 4

void task_with_mtx()
{
    for (int i = 0; i < LOOPS ; ++i) {
        lock_guard guard{mtx};
        ++counter;
    }
}

void task_with_shared_mtx_unique_lock()
{
    for (int i = 0; i < LOOPS ; ++i) {
        unique_lock guard{shared_mtx};
        ++counter;
    }
}

void task_with_shared_mtx_shared_lock()
{
    for (int i = 0; i < LOOPS ; ++i) {
        shared_lock guard{shared_mtx};
        ++counter;
    }
}

void task_with_shared_mtx_random_lock()
{
    mt19937 engine;
    uniform_int_distribution dist(0, 99);
    for (int i = 0; i < LOOPS ; ++i) {
        if (dist(engine) >= 90) {
            unique_lock guard{shared_mtx};
            ++counter;
        } else {
            shared_lock guard{shared_mtx};
            ++counter;
        }
    }
}

void test(const char* fn_name, void (*fn)())
{
    counter = 0;
    auto t1 = rdtsc();
    {
        std::vector<scoped_thread> threads;
        for (int i = 0; i < THRDS; ++i) {
            threads.emplace_back(fn);
        }
    }
    auto t2 = rdtsc();
    cout << fn_name << " takes " << (t2 - t1) / LOOPS
         << " cycles in each loop\n";
    cout << "Counter: " << counter << '\n';
}

#define TEST(fn) test(#fn, fn)

int main()
{
    TEST(task_with_mtx);
    TEST(task_with_shared_mtx_unique_lock);
    TEST(task_with_shared_mtx_shared_lock);
    TEST(task_with_shared_mtx_random_lock);
}

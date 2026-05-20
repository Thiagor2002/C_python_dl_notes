#include <chrono>
#include <iostream>

using namespace std;

int main()
{
    {
        auto t1 = chrono::steady_clock::now();
        cout << "Hello world\n";
        auto t2 = chrono::steady_clock::now();
        cout << chrono::duration_cast<chrono::nanoseconds>(t2 - t1).count()
             << " ns has elapsed\n";
    }
    {
        auto t1 = chrono::steady_clock::now();
        cout << "Hello world\n";
        auto t2 = chrono::steady_clock::now();
        cout << chrono::duration_cast<chrono::nanoseconds>(t2 - t1).count()
             << " ns has elapsed\n";
    }
}


#include <cstddef>      // std::size_t
#include <iostream>     // std::cout/endl
#include <list>         // std::list
#include <vector>       // std::vector

using namespace std;

template <typename T>
concept reservable = requires(T a) { a.reserve(1U); };

template <reservable C, typename T>
void append(C& container, const T* ptr, size_t size)
{
    cout << __PRETTY_FUNCTION__ << " (reservable)" << endl;
    container.reserve(container.size() + size);
    for (size_t i = 0; i < size; ++i) {
        container.push_back(ptr[i]);
    }
}

template <typename C, typename T>
void append(C& container, const T* ptr, size_t size)
{
    cout << __PRETTY_FUNCTION__ << endl;
    for (size_t i = 0; i < size; ++i) {
        container.push_back(ptr[i]);
    }
}

int main()
{
    list<int> lst;
    vector<int> vec;
    int a[] = {1, 2, 3, 4, 5};
    append(lst, data(a), size(a));
    append(vec, data(a), size(a));
}

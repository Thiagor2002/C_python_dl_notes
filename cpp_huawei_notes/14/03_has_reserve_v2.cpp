#include <cstddef>      // std::size_t
#include <iostream>     // std::cout/endl
#include <list>         // std::list
#include <type_traits>  // std::enable_if
#include <vector>       // std::vector

using namespace std;

template <typename T, typename = void_t<>>
struct has_reserve : false_type {};

template <typename T>
struct has_reserve<T, void_t<decltype(declval<T&>().reserve(1U))>>
    : true_type {};

template <typename C, typename T>
void _append(C& container, T* ptr, size_t size, true_type)
{
    cout << __PRETTY_FUNCTION__ << endl;
    container.reserve(container.size() + size);
    for (size_t i = 0; i < size; ++i) {
        container.push_back(ptr[i]);
    }
}

template <typename C, typename T>
void _append(C& container, T* ptr, size_t size, false_type)
{
    cout << __PRETTY_FUNCTION__ << endl;
    for (size_t i = 0; i < size; ++i) {
        container.push_back(ptr[i]);
    }
}

template <typename C, typename T>
void append(C& container, T* ptr, size_t size)
{
    _append(container, ptr, size,
            integral_constant<bool, has_reserve<C>::value>{});
}

int main()
{
    list<int> lst;
    vector<int> vec;
    int a[] = {1, 2, 3, 4, 5};
    append(lst, data(a), size(a));
    append(vec, data(a), size(a));
}

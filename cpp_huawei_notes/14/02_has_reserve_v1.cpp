#include <cstddef>      // std::size_t
#include <iostream>     // std::cout/endl
#include <list>         // std::list
#include <type_traits>  // std::enable_if
#include <vector>       // std::vector

using namespace std;

template <typename T>
struct has_reserve {
    struct good { char dummy; };
    struct bad { char dummy[2]; };
    template <class U, void (U::*)(size_t)>
    struct SFINAE {};
    template <class U>
    static good reserve(SFINAE<U, &U::reserve>*);
    template <class U>
    static bad reserve(...);
    static const bool value =
        sizeof(reserve<T>(nullptr)) == sizeof(good);
};

template <typename C, typename T>
enable_if_t<has_reserve<C>::value, void> append(C& container, T* ptr,
                                                size_t size)
{
    cout << __PRETTY_FUNCTION__ << endl;
    container.reserve(container.size() + size);
    for (size_t i = 0; i < size; ++i) {
        container.push_back(ptr[i]);
    }
}

template <typename C, typename T>
enable_if_t<!has_reserve<C>::value, void> append(C& container, T* ptr,
                                                 size_t size)
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

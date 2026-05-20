#include <cstddef>               // NULL
#include <iostream>              // std::cout
#include <boost/type_index.hpp>  // boost::typeindex::type_id

using namespace std;
using boost::typeindex::type_id;

template <typename T>
void foo(T)
{
    cout << type_id<T>() << '\n';
}

int main()
{
    foo(NULL);
}

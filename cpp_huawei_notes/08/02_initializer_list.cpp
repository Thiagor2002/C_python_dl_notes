#include <initializer_list>      // std::initializer_list
#include <iostream>              // std::cout/endl
#include <boost/type_index.hpp>  // boost::typeindex::type_id

using namespace std;
using boost::typeindex::type_id;

int main()
{
    auto lst1 = {1, 2, 3};
    cout << "Type of lst1 is " << type_id<decltype(lst1)>() << endl;
    // auto lst2 = {1, 2, 3.0}; // 不能编译
    initializer_list<double> lst2 = {1, 2, 3.0}; // 可以显示声明
    // initializer_list<int> lst3 = {1, 2, 3.0}; // 不允许窄化转换
    cout << "Type of lst2 is " << type_id<decltype(lst2)>() << endl;
}

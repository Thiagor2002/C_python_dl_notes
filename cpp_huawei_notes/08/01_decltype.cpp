#include <iostream>              // std::cout/endl
#include <boost/type_index.hpp>  // boost::typeindex::type_id_with_cvr

using namespace std;
using boost::typeindex::type_id_with_cvr;

int main()
{
    int a = 0;
    cout << "decltype(a): " << type_id_with_cvr<decltype(a)>() << endl;
    cout << "decltype((a)): " << type_id_with_cvr<decltype((a))>() << endl;
    cout << "decltype(a + a): " << type_id_with_cvr<decltype(a + a)>()
         << endl;
}

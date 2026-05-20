#include <iostream>
#include <cln/cln.h>

using namespace std;

template <typename E>
E my_mod(const E& lhs, const E& rhs)
{
    return lhs % rhs;
}

template <>
cln::cl_I my_mod<cln::cl_I>(const cln::cl_I& lhs, const cln::cl_I& rhs)
{
    return mod(lhs, rhs);
}

template <typename E>
E my_gcd(E a, E b)
{
    while (b != E(0)) {
        E r = my_mod(a, b);
        a = b;
        b = r;
    }
    return a;
}

int main()
{
    cln::cl_I a;
    cln::cl_I b;
    cin >> a >> b;
    cout << my_gcd(a, b) << endl;
}

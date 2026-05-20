#include <iostream>

using namespace std;

template <typename E>
E my_gcd(E a, E b)
{
    while (b != E(0)) {
        E r = a % b;
        a = b;
        b = r;
    }
    return a;
}

int main()
{
    long a;
    long b;
    cin >> a >> b;
    cout << my_gcd(a, b) << endl;
}

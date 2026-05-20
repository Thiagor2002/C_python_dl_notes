#include "availability_hack.h"  // Hack for weird restriction on macOS
#include <climits>              // INT_MIN
#include <iostream>             // std::cout/endl
#include <optional>             // std::optional

using namespace std;

optional<int> safe_divide(int i, int j)
{
    if (j == 0) {
        return {};
    } else if (i == INT_MIN && j == -1) {
        return {};
    } else if (i % j != 0) {
        return {};
    } else {
        return i / j;
    }
}

int main()
{
    int i;
    int j;
    cout << "Input two integers: ";
    cin >> i >> j;
    optional<int> result = safe_divide(i, j);
    if (result) {
        cout << i << " / " << j << " = " << *result << endl;
    } else {
        cout << "Error occurred\n";
    }
}

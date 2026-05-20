#include <iostream>
#include <vector>

using namespace std;

template <typename T>
int TypeValue = 0;

template <>
int TypeValue<int> = 1;

template <typename T>
int TypeValue<T*> = 9;

template <template <typename> class T, typename U>
int TypeValue<T<U>> = 99;

template <template <typename, typename> class T, typename U, typename V>
int TypeValue<T<U, V>> = 999;

int main()
{
    cout << TypeValue<char> << endl;
    cout << TypeValue<int> << endl;
    cout << TypeValue<char*> << endl;
    cout << TypeValue<vector<int>> << endl;
}

#include <iostream>
#include <string>
#include <tuple>
#include <vector>
#include "output_range.h"

using namespace std;

using my_tuple = tuple<int, string, string>;

int main()
{
    vector<my_tuple> vn{
        {1, "one", "un"},
        {2, "two", "deux"},
        {3, "three", "trois"}
    };

    cout << vn << endl;
    get<2>(vn[0]) = "une";
    cout << vn << endl;
}

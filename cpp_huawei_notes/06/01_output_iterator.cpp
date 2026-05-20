#include <algorithm>  // std::transform
#include <iostream>   // std::cout/endl
#include <iterator>   // std::back_inserter/ostream_iterator
#include <vector>     // std::vector

using namespace std;

int main()
{
    vector<int> in{1, 2, 3, 4, 5};
    vector<int> out;
    transform(in.begin(), in.end(), back_inserter(out),
              [](int x) { return x * x; });
    copy(out.begin(), out.end(), ostream_iterator<int>(cout, " "));
    cout << endl;
}

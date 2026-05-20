#include <iostream>
#include <vector>
#include "output_range.h"

using namespace std;

int main()
{
    vector<int> vi{1, 2, 3, 4, 5};
    std::cout << vi << std::endl;
    vi.insert(vi.begin(), 0);
    std::cout << vi << std::endl;
}

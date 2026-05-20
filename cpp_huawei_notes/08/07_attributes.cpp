#include <stdexcept>

using namespace std;

int test1(int c)
{
    int result = 0;
    switch (c) {
    case 2:
        ++result;
        [[fallthrough]];
    case 1:
        ++result;
        break;
    default:
        throw runtime_error("bad input");
    }
    return result;
}

[[nodiscard]] bool test2(int a)
{
    [[maybe_unused]] int b = a + 1;
    return true;
}

int main()
{
    test1(42);
    test2(42);
}

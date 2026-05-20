#include <iostream>

using namespace std;

class Obj {
public:
    Obj() = default;
    Obj(int n) : value_(n) {}
    int value() const { return value_; }

private:
    int value_;
};

int main()
{
    {
        Obj obj{};
        cout << obj.value() << endl;
    }
    {
        Obj obj{42};
        cout << obj.value() << endl;
    }
    {
        Obj obj;
        cout << obj.value() << endl;
    }
}

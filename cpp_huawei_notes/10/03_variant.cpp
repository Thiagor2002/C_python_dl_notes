#include "availability_hack.h"  // Hack for weird restriction on macOS
#include <iostream>             // std::cout/endl
#include <string>               // std::string
#include <variant>              // std::variant

using namespace std;

int main()
{
    variant<string, int, char> obj{"Hello world"};
    cout << obj.index() << ": " << get<string>(obj) << endl;
    obj = 42;
    cout << obj.index() << ": " << get<int>(obj) << endl;
    if (auto ptr = get_if<int>(&obj)) {
        cout << "Found int value " << *ptr << endl;
    }
    cout << get<string>(obj) << endl;  // Boom
}

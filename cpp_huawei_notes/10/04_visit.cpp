#include "availability_hack.h"  // Hack for weird restriction on macOS
#include <iostream>             // std::cout/endl
#include <string>               // std::string
#include <variant>              // std::variant

using namespace std;

// Helper for visit
template <class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

void output(const variant<string, int, char>& obj)
{
    visit(overloaded{
        [](const string& value) { cout << "s: " << value << '\n'; },
        [](int value) { cout << "i: " << value << '\n'; },
        [](char value) { cout << "c: " << value << '\n'; },
    }, obj);
}

int main()
{
    variant<string, int, char> obj{"Hello world"};
    output(obj);
    if (auto ptr = get_if<string>(&obj)) {
        cout << "Finding string is successful: " << *ptr << '\n';
    }
}

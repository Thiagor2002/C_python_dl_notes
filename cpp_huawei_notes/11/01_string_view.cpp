#include <iostream>     // std::cout/endl
#include <string>       //std::string
#include <string_view>  //std::string_view

using namespace std;

string generate_greeting(string_view name) {
    string result("Hi, ");
    result.append(name.data(), name.size());
    return result;
}

int main()
{
    cout << generate_greeting("Horatio") << endl;
    cout << generate_greeting("Horatio"s) << endl;
    cout << generate_greeting("Horatio"sv) << endl;
}

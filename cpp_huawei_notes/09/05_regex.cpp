#include <iostream>
#include <regex>
#include <string>

using namespace std;

int main()
{
    string line;
    regex pat("^Subject: (Re: |Fw: )*(.*)");

    while (cin) {
        getline(cin, line);
        smatch matches;
        if (regex_match(line, matches, pat)) {
            cout << matches[2] << endl;
        }
    }
}

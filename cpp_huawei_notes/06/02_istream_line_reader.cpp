#include <iostream>
#include <string>
#include "istream_line_reader.h"

using namespace std;

int main()
{
    for (const string& line : istream_line_reader(cin)) {
        // 示例循环体中仅进行简单输出
        cout << line << endl;
    }
}

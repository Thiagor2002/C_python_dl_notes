#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>

using namespace std;

int main()
{
    // 必须在初始化列表里写出 byte（但 vector 的模板参数可省略）
    vector<byte> v1{byte{0x42}, byte{0x61}, byte{0x64}};

    // 初始化列表里不需要写类型
    vector<uint8_t> v2{0x42, 0x61, 0x64};

    for (const auto& item : v1) {
        // cout << item << ' '; // 不能编译
        // 必须用 to_integer 或 static_cast 转换后才能输出
        cout << to_integer<unsigned>(item) << ' ';
    }
    cout << '\n';

    for (const auto& item : v2) {
        // 可以直接输出，当作字符而非无符号整数！
        cout << item << ' ';
    }
    cout << '\n';
}

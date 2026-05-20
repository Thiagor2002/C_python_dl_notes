#include <array>     // std::array
#include <cstddef>   // std::size_t
#include <iostream>  // std::cout
#include <utility>   // std::make_index_sequence

using namespace std;

constexpr int count_bits(unsigned char value)
{
    if (value == 0) {
        return 0;
    } else {
        return (value & 1) + count_bits(value >> 1);
    }
}

template <size_t... V>
constexpr auto get_bit_count(index_sequence<V...>)
{
    return std::array<unsigned char, sizeof...(V)>{
        static_cast<unsigned char>(count_bits(V))...};
}

auto bit_count = get_bit_count(make_index_sequence<256>());

int main()
{
    for (int i = 0; i < 16; ++i) {
        cout << static_cast<unsigned>(bit_count[i]) << '\n';
    }
}

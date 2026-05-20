#include <iostream>
#include <system_error>
#include <stdlib.h>
#include "nvwa/mmap_line_view.h"

int main(int argc, char* argv[])
{
    if (argc != 2) {
        std::cerr << "One file name should be provided\n";
        exit(1);
    }

    try {
        nvwa::mmap_line_view reader{argv[1]};
        for (auto& line : reader) {
            std::cout << line << '\n';
        }
    }
    catch (std::system_error& e) {
        std::cerr << e.what() << '\n';
        if (e.code() == std::errc::no_such_file_or_directory) {
            std::cerr << "Did you spell the file name correctly?\n";
        }
        exit(1);
    }
}

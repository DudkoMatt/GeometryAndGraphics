#include <iostream>
#include <cstring>

bool check_color_space(char *color_space) {
    if (color_space == nullptr)
        return false;
    return strcmp(color_space, "RGB") == 0 ||
    strcmp(color_space, "HSL") == 0 ||
    strcmp(color_space, "HSV") == 0 ||
    strcmp(color_space, "YCbCr.601") == 0 ||
    strcmp(color_space, "YCbCr.709") == 0 ||
    strcmp(color_space, "YCoCg") == 0 ||
    strcmp(color_space, "CMY") == 0;
}

int main(int argc, char *argv[]) {

    if (argc != 11) {
        std::cerr << "Wrong number of arguments. Syntax:\n<lab4>.exe -f <from_color_space> -t <to_color_space> -i <count> <input_file_name> -o <count> <output_file_name>\n";
        return 1;
    }

    char _f = 0, _t = 0, _i = 0, _o = 0;
    char *from_color_space = nullptr, *to_color_space = nullptr;
    char *file_in_name = nullptr, *file_out_name = nullptr;
    int i_count = 0, o_count = 0;

    for (int i = 1; i < argc; ++i) {
        if (strlen(argv[i]) == 2) {
            if (strncmp(argv[i], "-f", 2) == 0) {
                _f = 1;
                from_color_space = argv[++i];
            } else if (strncmp(argv[i], "-t", 2) == 0) {
                _t = 1;
                to_color_space = argv[++i];
            } else if (strncmp(argv[i], "-i", 2) == 0) {
                _i = 1;
                i_count = std::stoi(argv[++i]);
                file_in_name = argv[++i];
            } else if (strncmp(argv[i], "-o", 2) == 0) {
                _o = 1;
                o_count = std::stoi(argv[++i]);
                file_out_name = argv[++i];
            } else {
                fprintf(stderr, "Invalid argument: %s\n", argv[i]);
                return 1;
            }
        } else {
            fprintf(stderr, "Invalid argument: %s\n", argv[i]);
            return 1;
        }
    }
    
    if (_f + _t + _i + _o < 4 || (i_count != 1 && i_count != 3) || (o_count != 1 && o_count != 3)
            || !check_color_space(from_color_space) || !check_color_space(to_color_space)
            || file_in_name == nullptr || file_out_name == nullptr) {
        std::cerr << "Wrong arguments\n";
        return 1;
    }

    std::cout << "-f: " << from_color_space << std::endl;
    std::cout << "-t: " << to_color_space << std::endl;
    std::cout << "-i: " << i_count << " " << file_in_name << std::endl;
    std::cout << "-o: " << o_count << " " << file_out_name << std::endl;

    return 0;
}

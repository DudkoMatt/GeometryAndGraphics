#include <iostream>
#include <cstring>
#include <string>
#include "write_data_pnm_v2.h"

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

void add_channel(unsigned char *pix_data, int k_channel, int k_bytes_channel, const unsigned char *channel) {
    for (int i = 0; i < k_bytes_channel; ++i) {
        *(pix_data + i * 3 + k_channel) = *(channel + i);
    }
}

int main(int argc, char *argv[]) {

    // Часть 1: разбор аргументов командной строки

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

    // Часть 2: проверка на существование файла и чтение файла на ввод

    unsigned char *pix_data = nullptr;
    FILE *file_in = nullptr, *file_out = nullptr;

    char char_header = 0;
    int width = 0, height = 0;
    unsigned int max_value = 0;
    int k_bytes = 0;

    if (i_count == 1) {

        file_in = fopen(file_in_name, "rb");
        if (file_in == nullptr) {
            std::cerr << "Cannot open file to read: " << file_in_name << "\n";
            return 1;
        }

        read_header(char_header, width, height, max_value, file_in);
        if (char_header != '6') {
            std::cerr << "Expected PPM file format\n";
            free_data(file_in, file_out, pix_data);
            return 1;
        }

        k_bytes = 3 * width * height;

        pix_data = (unsigned char *) calloc(k_bytes, 1);
        if (!pix_data) {
            std::cerr << "Cannot allocate " << k_bytes << " bytes of memory\n";
            free_data(file_in, file_out, pix_data);
            return 1;
        }

        read_data(file_in, k_bytes, pix_data);

        fclose(file_in);

    } else {
        unsigned char *tmp_pix_data = nullptr;
        int prev_k_bytes = -1;

        for (int i = 1; i <= 3; ++i) {
            std::string tmp_file_name(file_in_name);
            std::string idx("_");
            idx.append(1, '0' + i);
            tmp_file_name.insert(tmp_file_name.begin() + tmp_file_name.find_last_of('.'), idx.begin(), idx.end());

            file_in = fopen(tmp_file_name.data(), "rb");

            if (file_in == nullptr) {
                std::cerr << "Cannot open file to read: " << file_in_name << "\n";
                return 1;
            }

            read_header(char_header, width, height, max_value, file_in);
            if (char_header != '5') {
                std::cerr << "Expected PGM file format\n";
                free_data(file_in, file_out, pix_data);
                return 1;
            }

            k_bytes = width * height;

            if (k_bytes != prev_k_bytes && prev_k_bytes != -1) {
                std::cerr << "Different amount of data in files\n";
                free_data(file_in, file_out, pix_data);
                if (tmp_pix_data)
                    free(tmp_pix_data);
                return 1;
            } else {
                prev_k_bytes = k_bytes;
            }
            
            if (i == 1) {
                pix_data = (unsigned char *) calloc(3 * k_bytes, 1);
                if (!pix_data) {
                    std::cerr << "Cannot allocate " << 3 * k_bytes << " bytes of memory\n";
                    free_data(file_in, file_out, pix_data);
                    return 1;
                }
                
                // Initialization
                for (int __i = 0; __i < k_bytes; ++__i) {
                    pix_data[__i] = 0;
                }

                tmp_pix_data = (unsigned char *) calloc(k_bytes, 1);
                if (!tmp_pix_data) {
                    std::cerr << "Cannot allocate " << k_bytes << " bytes of memory\n";
                    free_data(file_in, file_out, pix_data);
                    return 1;
                }
            }

            for (int __i = 0; __i < k_bytes; ++__i) {
                *(tmp_pix_data + i) = 0;
            }

            read_data(file_in, k_bytes, tmp_pix_data);
            add_channel(pix_data, i - 1, k_bytes, tmp_pix_data);

            fclose(file_in);
        }

        if (tmp_pix_data)
            free(tmp_pix_data);

        k_bytes *= 3;
    }


    // Test file input writing

    file_out = fopen(file_out_name, "wb");
    if (file_out == nullptr) {
        std::cerr << "Cannot open file to write: " << file_out_name << "\n";
        free_data(file_in, file_out, pix_data);
        return 1;
    }

    color::write_to_file(file_out, width, height, max_value, pix_data);
    free_data(file_in, file_out, pix_data);

    return 0;
}

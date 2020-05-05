#include <iostream>
#include "../Lab_04/write_data_pnm_v2.h"
#include "conversions_color_spaces.h"

unsigned char limit_brightness(long double brightness) {
    return (unsigned char) std::min(255.0l, std::max(0.0l, brightness));
}

void change_offset_multiply_all(unsigned char *pix_data, int all_bytes, int offset, long double multiplier) {
    for (int i = 0; i < all_bytes; ++i) {
        *(pix_data + i) = limit_brightness(((int) *(pix_data + i) - offset) * multiplier);
    }
}

void change_offset_multiply_Y(unsigned char *pix_data, int all_bytes, int offset, long double multiplier) {
    for (int i = 0; i < all_bytes; i += 3) {
        *(pix_data + i) = limit_brightness(((int) *(pix_data + i) - offset) * multiplier);
    }
}

int main(int argc, char *argv[]) {

    if (argc != 4 && argc != 6) {
        std::cerr << "Wrong number of arguments. Syntax:\n<lab5>.exe <file_in> <file_out> <conversion> [<offset> <multiplier>]\n";
        return 1;
    }

    const char *file_in_name = argv[1];
    const char *file_out_name = argv[2];

    unsigned conversion = argv[3][0] - '0';

    int offset = 0;
    long double multiplier = 0;
    if (argc == 6) {
        offset = std::stoi(argv[4]);
        multiplier = std::stold(argv[5]);
    }

    if (conversion > 5 || ((conversion == 0 || conversion == 1) && argc != 6) || (conversion > 1 && argc == 6)) {
        std::cerr << "Wrong arguments\n";
        return 1;
    }

    char char_header;
    int width, height;
    unsigned int max_value;

    FILE *file_in = fopen(file_in_name, "rb");

    if (file_in == nullptr) {
        std::cerr << "Cannot open file to read: " << file_in_name << "\n";
        return 1;
    }

    fscanf(file_in, "P%c\n%i %i\n%i\n", &char_header, &width, &height, &max_value);

    if (width <= 0 || height <= 0 || (char_header != '5' && char_header != '6')) {
        std::cerr << "Wrong file attributes\n";
        fclose(file_in);
        return 1;
    }


    // Проверка совместимости преобразования и формата файла
    if (char_header == '5' && conversion % 2 == 1) {
        std::cerr << "Cannot apply conversion for PGM file format\n";
        free_data(file_in);
        return 1;
    }


    FILE *file_out = nullptr;
    int k_bytes = height * width;

    if (char_header == '6')
        k_bytes *= 3;

    auto pix_data = (unsigned char *) calloc(k_bytes, 1);
    if (!pix_data) {
        std::cerr << "Cannot allocate " << k_bytes << " bytes of memory\n";
        free_data(file_in, file_out, pix_data);
        return 1;
    }

    read_data(file_in, k_bytes, pix_data);


    // Преобразования
    switch (conversion) {
        case 0:
            change_offset_multiply_all(pix_data, k_bytes, offset, multiplier);
            break;
        case 1:
            conversions::rgb_2_YCbCr_601(pix_data, k_bytes);
            change_offset_multiply_Y(pix_data, k_bytes, offset, multiplier);
            conversions::YCbCr_601_2_rgb(pix_data, k_bytes);
            break;
        case 2:
            // ToDO
            break;
        case 3:
            // ToDO
            break;
        case 4:
            // ToDO
            break;
        case 5:
            // ToDO
            break;
        default:
            break;
    }


    file_out = fopen(file_out_name, "wb");
    if (file_out == nullptr) {
        std::cerr << "Cannot open file to write: " << file_out_name << "\n";
        free_data(file_in, file_out, pix_data);
        return 1;
    }

    if (char_header == '5')
        gray::write_to_file(file_out, width, height, max_value, pix_data);
    else
        color::write_to_file(file_out, width, height, max_value, pix_data);

    free_data(file_in, file_out, pix_data);
    return 0;
}

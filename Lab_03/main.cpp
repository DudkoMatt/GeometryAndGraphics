#include <iostream>
#include <cmath>
#include "write_data_pnm.h"

void fill_vertical_line(int x, int width, int height, unsigned char color, unsigned char *pix_data) {
    for (int i = 0; i < height; ++i) {
        draw_pix(pix_data, width, x, i, color, 1);
//        pix_data[x + i * width] = color;
    }
}

void no_dithering(int width, int height, unsigned char *pix_data) {
    for (int i = 0; i < width; ++i) {
        fill_vertical_line(i, width, height, (unsigned char) std::round(255.0 * i / width), pix_data);
    }
}

void ordered_dithering() {

}

void random_dithering() {

}

void Floyd_Steinberg_dithering() {

}


int main(int argc, char *argv[]) {
    if (argc != 7 && argc != 6) {
        std::cerr
                << "Wrong number of arguments. Syntax:\n<lab>.exe file_in file_out gradient dithering bitness [gamma]";
        return 1;
    }

    const char *file_in_name = argv[1];
    const char *file_out_name = argv[2];

    unsigned gradient = argv[3][0] - '0';
    unsigned dithering = argv[4][0] - '0';
    unsigned bitness = argv[5][0] - '0';

    // ToDO: sRGB
    double gamma = 0;

    if (argc == 7)
        try {
            gamma = std::stod(argv[6]);
        } catch (std::invalid_argument &e) {
            std::cerr << "Cannot convert gamma from string to double\n";
            return 1;
        }

    // Если что-то пошло не так:
    if (gradient > 1 || dithering > 7 || bitness == 0 || bitness > 8 || gamma < 0) {
        std::cerr << "Wrong arguments";
        return 1;
    }

    // Считывание размеров из файла
    char char_header;
    int width, height;
    unsigned int max_value;

    FILE *file_in = fopen(file_in_name, "rb");

    if (file_in == nullptr) {
        std::cerr << "Cannot open file to read: " << file_in_name << "\n";
        return 1;
    }

    fscanf(file_in, "P%c\n%i %i\n%i\n", &char_header, &width, &height, &max_value);

    if (width <= 0 || height <= 0) {
        std::cerr << "Wrong file attributes\n";
        fclose(file_in);
        return 1;
    }


    // Открытие файла на выход и выделение памяти для данных

    FILE *file_out = nullptr;
    int k_bytes = height * width;

    auto pix_data = (unsigned char *) calloc(k_bytes, 1);
    if (!pix_data) {
        std::cerr << "Cannot allocate " << k_bytes << " bytes of memory\n";
        free_data(file_in, file_out, pix_data);
        return 1;
    }

    // Initialization
    for (int i = 0; i < k_bytes; ++i) {
        pix_data[i] = 0;
    }

    file_out = fopen(file_out_name, "wb");
    if (file_out == nullptr) {
        std::cerr << "Cannot open file to write: " << file_out_name << "\n";
        free_data(file_in, file_out, pix_data);
        return 1;
    }









    // ToDO: сейчас частичное решение
    if (gradient != 1 || dithering > 3 || gamma != 1.0) {
        std::cerr << "Only partial solution";
        free_data(file_in, file_out, pix_data);
        return 1;
    }


    // Основная часть программы

    if (dithering == 0) {
        no_dithering(width, height, pix_data);
    } else if (dithering == 1) {

    } else if (dithering == 2) {

    } else if (dithering == 3) {

    } /*else if (dithering == 4) {

    } else if (dithering == 5) {

    } else if (dithering == 6) {

    } else if (dithering == 7) {

    }*/

    else {
        std::cerr << "Not implemented for now";
    }

    write_to_file(file_out, char_header, width, height, max_value, pix_data);

    free_data(file_in, file_out, pix_data);
    return 0;
}

#include <iostream>
#include <cmath>
#include <vector>
#include "write_data_pnm.h"
#include <algorithm>
#include <string>

//#define FILE_OUTPUT
#define ENABLE_FILE_INPUT

// Debug output
#ifdef FILE_OUTPUT
#include <fstream>
#endif

const double Bayer_Matrix_double[8][8] = {
        {-0.5,     0.25,     -0.3125,  0.4375,   -0.453125, 0.296875,  -0.265625, 0.484375},
        {0.0,      -0.25,    0.1875,   -0.0625,  0.046875,  -0.203125, 0.234375,  -0.015625},
        {-0.375,   0.375,    -0.4375,  0.3125,   -0.328125, 0.421875,  -0.390625, 0.359375},
        {0.125,    -0.125,   0.0625,   -0.1875,  0.171875,  -0.078125, 0.109375,  -0.140625},
        {-0.46875, 0.28125,  -0.28125, 0.46875,  -0.484375, 0.265625,  -0.296875, 0.453125},
        {0.03125,  -0.21875, 0.21875,  -0.03125, 0.015625,  -0.234375, 0.203125,  -0.046875},
        {-0.34375, 0.40625,  -0.40625, 0.34375,  -0.359375, 0.390625,  -0.421875, 0.328125},
        {0.15625,  -0.09375, 0.09375,  -0.15625, 0.140625,  -0.109375, 0.078125,  -0.171875}
};

const int MATRIX_SIZE = 8;

const double Halftone_Matrix_double[4][4] = {
        {0.375,  0.75,   0.625,  0.1875},
        {0.6875, 0.9375, 0.8125, 0.4375},
        {0.5625, 0.875,  0.3125, 0.0625},
        {0.25,   0.5,    0.125,  0.0}
};


// Вывод в 0..255
unsigned char get_pix_color(int x, int y, int width, const unsigned char *pix_data) {
    if (pix_data == nullptr) {
        // Горизонтальный градиент
        // x in [0 .. width - 1]
        return (unsigned char) (255.0 * x / (width - 1));
    } else {
        return *(pix_data + y * width + x);
    }
}

// aka. find_nearest_color, округление
unsigned char change_bitness(unsigned bitness, unsigned char data) {
    unsigned char tmp = data & (((1u << bitness) - 1) << (8 - bitness));
    data = 0;

    for (unsigned i = 0; i < 8 / bitness + 1; ++i) {
        data = data | ((unsigned char) (tmp >> bitness * i));
    }

    return data;
}

void no_dithering_file(unsigned bitness, size_t k, unsigned char *pix_data) {
    if (bitness == 8) return;
    for (int i = 0; i < k; ++i) {
        *(pix_data + i) = change_bitness(bitness, *(pix_data + i));
    }
}

void fill_vertical_line(int x, int width, int height, unsigned char color, double gamma, unsigned char *pix_data) {
    for (int i = 0; i < height; ++i) {
        draw_pix(pix_data, width, x, i, color, gamma);
    }
}

void no_dithering(int width, int height, unsigned char *pix_data, double gamma, unsigned bitness) {
    for (int i = 0; i < width; ++i) {
        fill_vertical_line(
                i, width, height,
                change_bitness(
                        bitness,
                        get_pix_color(i, 0, width, nullptr)
                ),
                gamma, pix_data);
    }
}

void ordered_dithering(int width, int height, unsigned char *pix_data, double gamma, unsigned bitness,
                       unsigned char *pix_data_input = nullptr) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {

            double barrier_brightness = Bayer_Matrix_double[y % MATRIX_SIZE][x % MATRIX_SIZE];

            draw_pix(pix_data, width, x, y,
                     change_bitness(bitness, (unsigned char)

                             limit_brightness(get_pix_color(x, y, width, pix_data_input) + barrier_brightness * 255)

                     ),
                     gamma);


        }
    }
}

void random_dithering(int width, int height, unsigned char *pix_data, double gamma, unsigned bitness,
                      unsigned char *pix_data_input = nullptr) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {

            unsigned char curr_brightness_char = get_pix_color(x, y, width, pix_data_input);
            unsigned char nearest_palette_color;

            nearest_palette_color = change_bitness(bitness,
                                                   limit_brightness(curr_brightness_char +
                                                                    (((double) std::rand() / (RAND_MAX)) * 255 - 128)));

            draw_pix(pix_data, width, x, y,
                     nearest_palette_color,
                     gamma);

        }
    }
}

void Floyd_Steinberg_dithering(int width, int height, unsigned char *pix_data, double gamma, unsigned bitness,
                               unsigned char *pix_data_input = nullptr) {
    std::vector<std::vector<double>> errors = std::vector<std::vector<double>>(height, std::vector<double>(width, 0));

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {

            unsigned char curr_brightness_char = limit_brightness(
                    get_pix_color(x, y, width, pix_data_input) + errors[y][x]);
            unsigned char nearest_palette_color = change_bitness(bitness, curr_brightness_char);
            int err = curr_brightness_char - nearest_palette_color;

            // Вправо на данной строке
            if (x != width - 1) {
                errors[y][x + 1] += 7 * err / 16.0;
            }

            // Вниз на строку
            if (y != height - 1) {
                // Влево
                if (x != 0)
                    errors[y + 1][x - 1] += 3 * err / 16.0;

                // Центр
                errors[y + 1][x] += 5 * err / 16.0;

                // Вправо
                if (x != width - 1)
                    errors[y + 1][x + 1] += err / 16.0;
            }

            draw_pix(pix_data, width, x, y,
                     nearest_palette_color,
                     gamma);


        }
    }

}

void Jarvis_Judice_Ninke_dithering(int width, int height, unsigned char *pix_data, double gamma, unsigned bitness,
                                   unsigned char *pix_data_input = nullptr) {
    std::vector<std::vector<double>> errors = std::vector<std::vector<double>>(height, std::vector<double>(width, 0));

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {

            unsigned char curr_brightness_char = limit_brightness(
                    get_pix_color(x, y, width, pix_data_input) + errors[y][x]);
            unsigned char nearest_palette_color = change_bitness(bitness, curr_brightness_char);
            int err = curr_brightness_char - nearest_palette_color;

            const double k = 48.0;

            // Вправо на данной строке
            if (x < width - 1) {
                errors[y][x + 1] += 7 * err / k;
                if (x < width - 2)
                    errors[y][x + 2] += 5 * err / k;
            }

            // Вниз на строку
            if (y < height - 1) {
                // Влево на 2
                if (x >= 2)
                    errors[y + 1][x - 2] += 3 * err / k;

                // Влево на 1
                if (x != 0)
                    errors[y + 1][x - 1] += 5 * err / k;

                // Центр
                errors[y + 1][x] += 7 * err / k;

                // Вправо на 1
                if (x < width - 1)
                    errors[y + 1][x + 1] += 5 * err / k;

                // Вправо на 2
                if (x < width - 2)
                    errors[y + 1][x + 2] += 3 * err / k;

                // Если есть строка на 2 ниже
                if (y < height - 2) {
                    // Влево на 2
                    if (x >= 2)
                        errors[y + 2][x - 2] += 1 * err / k;

                    // Влево на 1
                    if (x != 0)
                        errors[y + 2][x - 1] += 3 * err / k;

                    // Центр
                    errors[y + 2][x] += 5 * err / k;

                    // Вправо на 1
                    if (x < width - 1)
                        errors[y + 2][x + 1] += 3 * err / k;

                    // Вправо на 2
                    if (x < width - 2)
                        errors[y + 2][x + 2] += 1 * err / k;
                }
            }

            draw_pix(pix_data, width, x, y,
                     nearest_palette_color,
                     gamma);


        }
    }
}

void Sierra_3_dithering(int width, int height, unsigned char *pix_data, double gamma, unsigned bitness,
                        unsigned char *pix_data_input = nullptr) {
    std::vector<std::vector<double>> errors = std::vector<std::vector<double>>(height, std::vector<double>(width, 0));

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {

            unsigned char curr_brightness_char = limit_brightness(
                    get_pix_color(x, y, width, pix_data_input) + errors[y][x]);
            unsigned char nearest_palette_color = change_bitness(bitness, curr_brightness_char);
            int err = curr_brightness_char - nearest_palette_color;

            const double k = 32.0;
            const double delta_err = err / k;

            // Вправо на данной строке
            if (x < width - 1) {
                errors[y][x + 1] += 5 * delta_err;
                if (x < width - 2)
                    errors[y][x + 2] += 3 * delta_err;
            }

            // Вниз на строку
            if (y < height - 1) {
                // Влево на 2
                if (x >= 2)
                    errors[y + 1][x - 2] += 2 * delta_err;

                // Влево на 1
                if (x != 0)
                    errors[y + 1][x - 1] += 4 * delta_err;

                // Центр
                errors[y + 1][x] += 5 * delta_err;

                // Вправо на 1
                if (x < width - 1)
                    errors[y + 1][x + 1] += 4 * delta_err;

                // Вправо на 2
                if (x < width - 2)
                    errors[y + 1][x + 2] += 2 * delta_err;

                // Если есть строка на 2 ниже
                if (y < height - 2) {
                    // Влево на 1
                    if (x != 0)
                        errors[y + 2][x - 1] += 2 * delta_err;

                    // Центр
                    errors[y + 2][x] += 3 * delta_err;

                    // Вправо на 1
                    if (x < width - 1)
                        errors[y + 2][x + 1] += 2 * delta_err;
                }
            }

            draw_pix(pix_data, width, x, y,
                     nearest_palette_color,
                     gamma);

        }
    }
}

void Atkinson_dithering(int width, int height, unsigned char *pix_data, double gamma, unsigned bitness,
                        unsigned char *pix_data_input = nullptr) {
    std::vector<std::vector<double>> errors = std::vector<std::vector<double>>(height, std::vector<double>(width, 0));

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {

            unsigned char curr_brightness_char = limit_brightness(
                    get_pix_color(x, y, width, pix_data_input) + errors[y][x]);
            unsigned char nearest_palette_color = change_bitness(bitness, curr_brightness_char);
            int err = curr_brightness_char - nearest_palette_color;

            const double k = 8.0;
            const double delta_err = err / k;

            // Вправо на данной строке
            if (x < width - 1) {
                errors[y][x + 1] += delta_err;
                if (x < width - 2)
                    errors[y][x + 2] += delta_err;
            }

            // Вниз на строку
            if (y < height - 1) {
                // Влево на 1
                if (x != 0)
                    errors[y + 1][x - 1] += delta_err;

                // Центр
                errors[y + 1][x] += delta_err;

                // Вправо на 1
                if (x < width - 1)
                    errors[y + 1][x + 1] += delta_err;

                // Если есть строка на 2 ниже
                if (y < height - 2) {
                    // Центр
                    errors[y + 2][x] += delta_err;
                }
            }

            draw_pix(pix_data, width, x, y,
                     nearest_palette_color,
                     gamma);


        }
    }
}

unsigned char calc_value(unsigned char pattern, unsigned bitness) {
    return change_bitness(bitness, pattern << (8 - bitness));
}

unsigned char
find_nearest_palette_color(unsigned bitness, unsigned char current_color, unsigned char barrier_brightness) {
    // Return color in [0..255]

    if (current_color <= barrier_brightness)
        return 0;
    else {
        unsigned char _pattern = (unsigned) current_color >> (8 - bitness);
        if (calc_value(_pattern, bitness) <= barrier_brightness)
            _pattern++;

        return calc_value(_pattern, bitness);
    }
}

void Halftone_dithering(int width, int height, unsigned char *pix_data, double gamma, unsigned bitness,
                        unsigned char *pix_data_input = nullptr) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {

            draw_pix(pix_data, width, x, y,
                     find_nearest_palette_color(bitness,
                                                get_pix_color(x, y, width, pix_data_input),
                                                (unsigned char) (255 * Halftone_Matrix_double[y % 4][x % 4])),
                     gamma);


        }
    }
}

int main(int argc, char *argv[]) {

    if (argc != 7 && argc != 6) {
        std::cerr
                << "Wrong number of arguments. Syntax:\n<lab>.exe file_in file_out gradient dithering bitness [gamma]\n";
        return 1;
    }

    const char *file_in_name = argv[1];
    const char *file_out_name = argv[2];

    unsigned gradient = argv[3][0] - '0';
    unsigned dithering = argv[4][0] - '0';
    unsigned bitness = argv[5][0] - '0';

    double gamma = 0;  // if gamma = 0, sRGB will be used

    if (argc == 7)
        try {
            gamma = std::stod(argv[6]);
        } catch (std::invalid_argument &e) {
            std::cerr << "Cannot convert gamma from string to double\n";
            return 1;
        }

    // Если что-то пошло не так:
    if (gradient > 1 || dithering > 7 || bitness == 0 || bitness > 8 || gamma < 0) {
        std::cerr << "Wrong arguments\n";
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

    // Основная часть программы


#ifndef ENABLE_FILE_INPUT
    if (gradient == 0) {
        std::cout << "File input disabled\n";
        free_data(file_in, file_out, pix_data);
        return 1;
    }
#endif

#ifdef ENABLE_FILE_INPUT

    if (gradient == 0) {
        // Читаем из файла

        auto input_pix_data = (unsigned char *) calloc(k_bytes, 1);
        if (!input_pix_data) {
            std::cerr << "Cannot allocate " << k_bytes << " bytes of memory\n";
            free_data(file_in, file_out, pix_data);
            return 1;
        }

        int bytes_read = fread(input_pix_data, 1, k_bytes, file_in);

        if (bytes_read < k_bytes) {
            std::cout << "Can't read all data:\n";
            std::cout << "Expected " << k_bytes << " bytes, but only " << bytes_read << " were read\n";
            free_data(file_in, file_out, input_pix_data);
            free(input_pix_data);
            return 1;
        }

        long current_pos_in_file = ftell(file_in);
        fseek(file_in, 0, SEEK_END);
        if (current_pos_in_file != ftell(file_in)) {
            std::cout << "Error: file contains more data than expected\n";
            free_data(file_in, file_out, input_pix_data);
            free(input_pix_data);
            return 1;
        }

//        decode_gamma_from_file(input_pix_data, width * height, gamma);

        if (dithering == 0) {
            // no_dithering

            no_dithering_file(bitness, width * height, input_pix_data);

            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    draw_pix(pix_data, width, x, y,
                             *(input_pix_data + y * width + x),
                             gamma);
                }
            }


        } else if (dithering == 1) {
            ordered_dithering(width, height, pix_data, gamma, bitness, input_pix_data);
        } else if (dithering == 2) {
            random_dithering(width, height, pix_data, gamma, bitness, input_pix_data);
        } else if (dithering == 3) {
            Floyd_Steinberg_dithering(width, height, pix_data, gamma, bitness, input_pix_data);
        } else if (dithering == 4) {
            Jarvis_Judice_Ninke_dithering(width, height, pix_data, gamma, bitness, input_pix_data);
        } else if (dithering == 5) {
            Sierra_3_dithering(width, height, pix_data, gamma, bitness, input_pix_data);
        } else if (dithering == 6) {
            Atkinson_dithering(width, height, pix_data, gamma, bitness, input_pix_data);
        } else if (dithering == 7) {
            Halftone_dithering(width, height, pix_data, gamma, bitness, input_pix_data);
        }

        free(input_pix_data);

    } else {
#endif
        // Горизонтальный градиент
        if (dithering == 0) {
            no_dithering(width, height, pix_data, gamma, bitness);
        } else if (dithering == 1) {
            ordered_dithering(width, height, pix_data, gamma, bitness);
        } else if (dithering == 2) {
            random_dithering(width, height, pix_data, gamma, bitness);
        } else if (dithering == 3) {
            Floyd_Steinberg_dithering(width, height, pix_data, gamma, bitness);
        } else if (dithering == 4) {
            Jarvis_Judice_Ninke_dithering(width, height, pix_data, gamma, bitness);
        } else if (dithering == 5) {
            Sierra_3_dithering(width, height, pix_data, gamma, bitness);
        } else if (dithering == 6) {
            Atkinson_dithering(width, height, pix_data, gamma, bitness);
        } else if (dithering == 7) {
            Halftone_dithering(width, height, pix_data, gamma, bitness);
        }

#ifdef ENABLE_FILE_INPUT
    }
#endif

    write_to_file(file_out, char_header, width, height, max_value, pix_data);


#ifdef FILE_OUTPUT
    // Debug output
    std::ofstream out("debug.txt");


    for (int i = 0; i < width * 1; ++i) {
        out << (int) *(pix_data + i) << " ";
    }
#endif

    free_data(file_in, file_out, pix_data);
    return 0;
}

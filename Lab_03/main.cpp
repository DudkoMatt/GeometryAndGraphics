#include <iostream>
#include <cmath>
#include <vector>
#include "write_data_pnm.h"
#include <algorithm>
#include <string>

#define DEBUG
//#define FILE_OUTPUT
#define ENABLE_FILE_INPUT

// ToDO: debug
#ifdef FILE_OUTPUT
#include <fstream>
#endif

const int Bayer_Matrix[8][8] = {
        {0,  48, 12, 60, 3,  51, 15, 63},
        {32, 16, 44, 28, 35, 19, 47, 31},
        {8,  56, 4,  52, 11, 59, 7,  55},
        {40, 24, 36, 20, 43, 27, 39, 23},
        {2,  50, 14, 62, 1,  49, 13, 61},
        {34, 18, 46, 30, 33, 17, 45, 29},
        {10, 58, 6,  54, 9,  57, 5,  53},
        {42, 26, 38, 22, 41, 25, 37, 21}
};

const int MATRIX_SIZE = 8;


const int Halftone_Matrix[4][4] = {
        {6,  12, 10, 3},
        {11, 15, 13, 7},
        {9,  14, 5,  1},
        {4,  8,  2,  0}
};


double get_pix_color(int x, int y, int width, const unsigned char *pix_data, double error) {
    if (pix_data == nullptr) {
        // Горизонтальный градиент
        // x in [0 .. width - 1]
        return (double) x / (width - 1) + error / 255.0;
    } else {
        return (*(pix_data + y * width + x) + error) / 255.0;
    }
}


unsigned char change_bitness(unsigned bitness, unsigned char data) {
    unsigned char tmp = data & (((1u << bitness) - 1) << (8 - bitness));
    data = 0;

    for (unsigned i = 0; i < 8 / bitness + 1; ++i) {
        data = data | ((unsigned char) (tmp >> bitness * i));
    }

    return data;
}


void change_bitness(unsigned bitness, size_t k, unsigned char *pix_data) {
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
        fill_vertical_line(i, width, height, change_bitness(bitness, (unsigned char) std::round(255.0 *
                                                                                                        change_pix_gamma_to_print(
                                                                                                                (double) i /
                                                                                                                width, gamma))),
                           gamma, pix_data);
    }
}

// Для ordered_dithering (8 x 8)
unsigned char find_nearest_palette_color(unsigned bitness, double pix_data, double barrier_brightness) {
    // Return color in [0..255]

    if (pix_data <= barrier_brightness)
        return 0;
    else {
        unsigned char current_color = (unsigned char) (pix_data * 255);

        while (current_color < 255 && current_color != change_bitness(bitness, current_color))
            current_color++;

        return current_color;
    }
}

// ToDO: исправить ошибку
void ordered_dithering(int width, int height, unsigned char *pix_data, double gamma, unsigned bitness, unsigned char *pix_data_input = nullptr) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {

            double barrier_brightness =
                    (Bayer_Matrix[y % MATRIX_SIZE][x % MATRIX_SIZE]) / ((double) MATRIX_SIZE * MATRIX_SIZE);

            double curr_brightness = change_pix_gamma_to_print(std::min(1.0, std::max(0.0,
                    get_pix_color(x, y, width, pix_data_input, 0) + (barrier_brightness - 0.5) / 2.0
            )), gamma);

            draw_pix(pix_data, width, x, y,
                     find_nearest_palette_color(bitness, curr_brightness, barrier_brightness),
                     gamma);


        }
    }
}


void Halftone_dithering(int width, int height, unsigned char *pix_data, double gamma, unsigned bitness, unsigned char *pix_data_input = nullptr) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {

            double barrier_brightness = (Halftone_Matrix[y % 4][x % 4]) / 16.0;

            double curr_brightness = change_pix_gamma_to_print(get_pix_color(x, y, width, pix_data_input, 0), gamma);

            draw_pix(pix_data, width, x, y,
                     find_nearest_palette_color(bitness, curr_brightness, barrier_brightness),
                     gamma);


        }
    }
}

// Для Floyd_Steinberg_dithering
// ToDO: сделать более быстрым подбор
unsigned char find_nearest_palette_color(unsigned bitness, double pix_data) {

    // pix_data in [0..255]

    int last_less = 0;
    int i = 0;
    for (i = 0; i <= 255; ++i) {
        if (change_bitness(bitness, i) < pix_data)
            last_less = change_bitness(bitness, i);
        else if (change_bitness(bitness, i) == pix_data)
            return change_bitness(bitness, i);
        else
            break;
    }

    // last_less < pix_data < i

    if (last_less == 255)
        return last_less;

    if (std::abs(pix_data - last_less) < std::abs(change_bitness(bitness, i) - pix_data)) {
        return last_less;
    } else
        return change_bitness(bitness, i);

}

void Floyd_Steinberg_dithering(int width, int height, unsigned char *pix_data, double gamma, unsigned bitness, unsigned char *pix_data_input = nullptr) {

    std::vector<std::vector<double>> errors = std::vector<std::vector<double>>(height, std::vector<double>(width, 0));

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {

#ifdef ENABLE_FILE_INPUT

            double curr_brightness = change_pix_gamma_to_print(get_pix_color(x, y, width, pix_data_input, errors[y][x]), gamma);
            unsigned char curr_brightness_char = std::min(std::max((int) (curr_brightness * 255), 0), 255);

#else

            unsigned char curr_brightness_char = std::max(std::min((int) (x * 255.0 / (width - 1) + errors[y][x]), 255), 0);

#endif

            unsigned char nearest_palette_color = find_nearest_palette_color(bitness,
                                                                             curr_brightness_char);
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

// ToDO:
void Jarvis_Judice_Ninke_dithering(int width, int height, unsigned char *pix_data, double gamma, unsigned bitness, unsigned char *pix_data_input = nullptr) {
    std::vector<std::vector<double>> errors = std::vector<std::vector<double>>(height, std::vector<double>(width, 0));

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {

            double curr_brightness = change_pix_gamma_to_print(get_pix_color(x, y, width, pix_data_input, errors[y][x]), gamma);
            unsigned char curr_brightness_char = std::min(std::max((int) (curr_brightness * 255), 0), 255);

            unsigned char nearest_palette_color = find_nearest_palette_color(bitness,
                                                                             curr_brightness_char);
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

// ToDO: Вопрос random in (0..1] или [-128 .. 128]?
void random_dithering(int width, int height, unsigned char *pix_data, double gamma, unsigned bitness, unsigned char *pix_data_input = nullptr) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {

            double curr_brightness = change_pix_gamma_to_print(get_pix_color(x, y, width, pix_data_input, 0), gamma);
            unsigned char curr_brightness_char = std::min(std::max((int) (curr_brightness * 255), 0), 255);

            unsigned char nearest_palette_color = find_nearest_palette_color(bitness, curr_brightness_char +
                                                                                      (((double) std::rand() /
                                                                                        (RAND_MAX)) * 255 - 128));

            draw_pix(pix_data, width, x, y,
                     nearest_palette_color,
                     gamma);


        }
    }
}

// ToDO:
void Atkinson_dithering(int width, int height, unsigned char *pix_data, double gamma, unsigned bitness, unsigned char *pix_data_input = nullptr) {
    std::vector<std::vector<double>> errors = std::vector<std::vector<double>>(height, std::vector<double>(width, 0));

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {

            double curr_brightness = change_pix_gamma_to_print(get_pix_color(x, y, width, pix_data_input, errors[y][x]), gamma);
            unsigned char curr_brightness_char = std::min(std::max((int) (curr_brightness * 255), 0), 255);

            unsigned char nearest_palette_color = find_nearest_palette_color(bitness,
                                                                             curr_brightness_char);
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

// ToDO:
void Sierra_3_dithering(int width, int height, unsigned char *pix_data, double gamma, unsigned bitness, unsigned char *pix_data_input = nullptr) {
    std::vector<std::vector<double>> errors = std::vector<std::vector<double>>(height, std::vector<double>(width, 0));

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {

            double curr_brightness = change_pix_gamma_to_print(get_pix_color(x, y, width, pix_data_input, errors[y][x]), gamma);
            unsigned char curr_brightness_char = std::min(std::max((int) (curr_brightness * 255), 0), 255);

            unsigned char nearest_palette_color = find_nearest_palette_color(bitness,
                                                                             curr_brightness_char);
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


#ifndef DEBUG

    if (gradient != 1 || dithering > 3 /*|| gamma != 1.0*/) {
        std::cerr << "Only partial solution";
        free_data(file_in, file_out, pix_data);
        return 1;
    }

#endif

    // Основная часть программы


#ifndef ENABLE_FILE_INPUT
    if (gradient == 0) {
        std::cout << "File input disabled";
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

        if (dithering == 0) {
            // no_dithering

            change_bitness(bitness, width * height, input_pix_data);

            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    draw_pix(pix_data, width, x, y,
                            change_pix_gamma_to_print(*(input_pix_data + y * width + x), gamma),
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

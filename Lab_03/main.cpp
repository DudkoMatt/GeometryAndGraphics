#include <iostream>
#include <cmath>
#include <vector>
#include "write_data_pnm.h"


#define DEBUG

// ToDO: debug
#include <fstream>


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


// ToDO: обработка битности внутри основной программы
void change_bitness(unsigned bitness, size_t k, unsigned char *pix_data) {
    if (bitness == 8) return;
    for (int i = 0; i < k; ++i) {
        unsigned char tmp = *(pix_data + i) & (((1u << bitness) - 1) << (8 - bitness));
        *(pix_data + i) = 0;

        for (unsigned j = 0; j < 8 / bitness + 1; ++j) {
            *(pix_data + j) = *(pix_data + j) | ((unsigned char) (tmp >> bitness * j));
        }
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

void fill_vertical_line(int x, int width, int height, unsigned char color, double gamma, unsigned char *pix_data) {
    for (int i = 0; i < height; ++i) {
        draw_pix(pix_data, width, x, i, color, gamma);
    }
}

void no_dithering(int width, int height, unsigned char *pix_data, double gamma, unsigned bitness) {
    for (int i = 0; i < width; ++i) {
        fill_vertical_line(i, width, height, change_bitness(bitness, (unsigned char) std::round(255.0 * i / width)),
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


//    int last_less = 0;
//    int i = 0;
//    for (i = 0; i <= 255; ++i) {
//        if (change_bitness(bitness, i) < pix_data)
//            last_less = change_bitness(bitness, i);
//        else if (change_bitness(bitness, i) == pix_data)
//            return change_bitness(bitness, i);
//        else
//            break;
//    }
//
//    // last_less < pix_data < i
//
//    if (last_less == 255)
//        return last_less;
//
//    if (pix_data - last_less < i - pix_data) {
//        return last_less;
//    } else
//        return change_bitness(bitness, i);
}


double change_pix_gamma(double _brightness, double gamma) {
    // Гамма коррекция:
    if (gamma > 0) {
        _brightness = std::pow(_brightness, gamma);
    } else {
        // ToDO: sRGB
    }

    return _brightness;
}

double change_pix_gamma(unsigned char pix_data, double gamma) {
    double _brightness = pix_data / 255.0;

    // Гамма коррекция:
    if (gamma > 0) {
        _brightness = std::pow(_brightness, gamma);
    } else {
        // ToDO: sRGB
    }

    return _brightness;
}

void ordered_dithering(int width, int height, unsigned char *pix_data, double gamma, unsigned bitness) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {

            // Black and white only --> no
//            double _color = std::pow((double) x / width, gamma);
//
//            if (std::pow((double) x / width, gamma) > Bayer_Matrix[y % MATRIX_SIZE][x % MATRIX_SIZE] / 64.0) {
//                *(pix_data + width * y + x) = 255;
//            } else {
//                *(pix_data + width * y + x) = 0;
//            }


            double barrier_brightness =
                    (Bayer_Matrix[y % MATRIX_SIZE][x % MATRIX_SIZE]) / ((double) MATRIX_SIZE * MATRIX_SIZE);
//            int barrier_brightness_digit = barrier_brightness * 255;

            double curr_brightness = change_pix_gamma((double) x / width, gamma);

//            unsigned char curr_brightness_char = curr_brightness * 255;

//            unsigned char _brightness = find_nearest_palette_color(bitness, curr_brightness, barrier_brightness);

            draw_pix(pix_data, width, x, y,
                     find_nearest_palette_color(bitness, curr_brightness, barrier_brightness),
                     gamma);


        }
    }
}

// Для Floyd_Steinberg_dithering
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

void Floyd_Steinberg_dithering(int width, int height, unsigned char *pix_data, double gamma, unsigned bitness) {

    std::vector<std::vector<double>> errors = std::vector<std::vector<double>>(height, std::vector<double>(width, 0));

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {

            double curr_brightness = change_pix_gamma((double) x / width, gamma);
            unsigned char curr_brightness_char = (unsigned char) (curr_brightness * 255);

            unsigned char nearest_palette_color = find_nearest_palette_color(bitness,
                                                                             curr_brightness_char + errors[y][x]);
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


void Jarvis_Judice_Ninke_dithering(int width, int height, unsigned char *pix_data, double gamma, unsigned bitness) {
    std::vector<std::vector<double>> errors = std::vector<std::vector<double>>(height, std::vector<double>(width, 0));

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {

            double curr_brightness = change_pix_gamma((double) x / width, gamma);
            unsigned char curr_brightness_char = (unsigned char) (curr_brightness * 255);

            unsigned char nearest_palette_color = find_nearest_palette_color(bitness,
                                                                             curr_brightness_char + errors[y][x]);
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
void random_dithering(int width, int height, unsigned char *pix_data, double gamma, unsigned bitness) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {

            double curr_brightness = change_pix_gamma((double) x / width, gamma);
            unsigned char curr_brightness_char = (unsigned char) (curr_brightness * 255);

            unsigned char nearest_palette_color = find_nearest_palette_color(bitness, curr_brightness_char +
                                                                                      (((double) std::rand() /
                                                                                        (RAND_MAX)) * 255 - 128));

            draw_pix(pix_data, width, x, y,
                     nearest_palette_color,
                     gamma);


        }
    }
}


void Atkinson_dithering(int width, int height, unsigned char *pix_data, double gamma, unsigned bitness) {
    std::vector<std::vector<double>> errors = std::vector<std::vector<double>>(height, std::vector<double>(width, 0));

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {

            double curr_brightness = change_pix_gamma((double) x / width, gamma);
            unsigned char curr_brightness_char = (unsigned char) (curr_brightness * 255);

            unsigned char nearest_palette_color = find_nearest_palette_color(bitness,
                                                                             curr_brightness_char + errors[y][x]);
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


int main(int argc, char *argv[]) {


// ToDO: Debug. Вывод таблицы
/*
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            std::cout << (Bayer_Matrix[y % MATRIX_SIZE][x % MATRIX_SIZE] < 10 ? " " : "") << Bayer_Matrix[y % MATRIX_SIZE][x % MATRIX_SIZE] << " ";
        }
        std::cout << std::endl;
    }

    std::cout << std::endl;std::cout << std::endl;std::cout << std::endl;

    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            std::cout << ((Bayer_Matrix[y % MATRIX_SIZE][x % MATRIX_SIZE] + 1) /
                          ((double) MATRIX_SIZE * MATRIX_SIZE) -
                          0.5) << " ";
        }
        std::cout << std::endl;
    }

    return 0;
*/

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

    // ToDO: сейчас частичное решение
    if (gradient != 1 || dithering > 3 /*|| gamma != 1.0*/) {
        std::cerr << "Only partial solution";
        free_data(file_in, file_out, pix_data);
        return 1;
    }

#endif

    // Основная часть программы


/*    if (gradient == 0) {
        // Читаем из файла

        int bytes_read = fread(pix_data, 1, k_bytes, file_in);

        if (bytes_read < k_bytes) {
            std::cout << "Can't read all data:\n";
            std::cout << "Expected " << k_bytes << " bytes, but only " << bytes_read << " were read\n";
            free_data(file_in, file_out, pix_data);
            return 1;
        }

        long current_pos_in_file = ftell(file_in);
        fseek(file_in, 0, SEEK_END);
        if (current_pos_in_file != ftell(file_in)) {
            std::cout << "Error: file contains more data than expected\n";
            free_data(file_in, file_out, pix_data);
            return 1;
        }

        change_bitness(bitness, width * height, pix_data);

        // ToDO

        if (dithering == 0) {
            // no_dithering

            // blank branch

        } else if (dithering == 1) {
            // ToDO
            // ordered_dithering
        } else if (dithering == 2) {
            // ToDO
            // random_dithering
        } else if (dithering == 3) {
            // ToDO
            // Floyd_Steinberg_dithering
        } else if (dithering == 4) {

        } else if (dithering == 5) {

        } else if (dithering == 6) {

        } else if (dithering == 7) {

        }

        else {
            std::cerr << "Not implemented for now";
        }


    } else {*/

    // Горизонтальный градиент

    if (dithering == 0) {
        no_dithering(width, height, pix_data, gamma, bitness);
    } else if (dithering == 1) {
        ordered_dithering(width, height, pix_data, gamma, bitness);
    } else if (dithering == 2) {
        // ToDO
        random_dithering(width, height, pix_data, gamma, bitness);
    } else if (dithering == 3) {
        // ToDO
        Floyd_Steinberg_dithering(width, height, pix_data, gamma, bitness);
    } else if (dithering == 4) {
        // ToDO
        Jarvis_Judice_Ninke_dithering(width, height, pix_data, gamma, bitness);
    } else if (dithering == 5) {

        // ToDO
        std::cout << "Sierra (Sierra-3): Not implemented for now" << std::endl;


    } else if (dithering == 6) {
        // ToDO
        Atkinson_dithering(width, height, pix_data, gamma, bitness);
    } else if (dithering == 7) {

        // ToDO
        std::cout << "Halftone (4x4, orthogonal): Not implemented for now" << std::endl;


    } else {
        std::cerr << "Wrong argument";
        return 1;
    }
//    }



    write_to_file(file_out, char_header, width, height, max_value, pix_data);


    // Debug output
//    std::ofstream out("debug.txt");
//
//
//    for (int i = 0; i < width * 1; ++i) {
//        out << (int) *(pix_data + i) << " ";
//    }


    free_data(file_in, file_out, pix_data);
    return 0;
}

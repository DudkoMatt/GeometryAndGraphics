#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
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

// RGB
void change_offset_multiply_Y(unsigned char *pix_data, int all_bytes, int offset, long double multiplier) {
    for (int i = 0; i < all_bytes; i += 3) {
        *(pix_data + i) = limit_brightness(((int) *(pix_data + i) - offset) * multiplier);
    }
}

// Gray
void change_offset_multiply_Y_gray(unsigned char *pix_data, int all_bytes, int offset, long double multiplier) {
    for (int i = 0; i < all_bytes; ++i) {
        *(pix_data + i) = limit_brightness(((int) *(pix_data + i) - offset) * multiplier);
    }
}


void
count_all_brightnesses_RGB(const unsigned char *pix_data, int all_bytes, std::vector<long long> &nums) {
    for (int i = 0; i < all_bytes; ++i) {
        nums[*(pix_data + i)] += 1;
    }
}

void
count_all_brightnesses_YCbCr(const unsigned char *pix_data, int all_bytes, char char_header, std::vector<long long> &nums) {
    if (char_header == '5') {
        // Gray
        for (int i = 0; i < all_bytes; ++i) {
            nums[*(pix_data + i)] += 1;
        }
    } else {
        // RGB -> YCbCr
        for (int i = 0; i < all_bytes; i += 3) {
            nums[*(pix_data + i)] += 1;
        }
    }
}

int main(int argc, char *argv[]) {

    if (argc != 4 && argc != 6) {
        std::cerr
                << "Wrong number of arguments. Syntax:\n<lab5>.exe <file_in> <file_out> <conversion> [<offset> <multiplier>]\n";
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

    if (conversion > 5 || ((conversion == 0 || conversion == 1) && argc != 6) || (conversion > 1 && argc == 6)
        || (argc == 6 && (std::abs(offset) > 255 || multiplier < 0 || multiplier - 1.0l / 255 < 0.001l
        || multiplier - 255.0l > 0.001l))) {
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

    if (conversion % 2 == 1 && char_header == '6') {
        conversions::rgb_2_YCbCr_601(pix_data, k_bytes);
    }

    // Преобразования
    if (conversion == 0) {
        change_offset_multiply_all(pix_data, k_bytes, offset, multiplier);
    } else if (conversion == 1) {
        if (char_header == '5')
            change_offset_multiply_Y_gray(pix_data, k_bytes, offset, multiplier);
        else
            change_offset_multiply_Y(pix_data, k_bytes, offset, multiplier);
    } else {
        std::vector<long long> nums_brightnesses = std::vector<long long>(256, 0);
        if (conversion % 2 == 0)
            count_all_brightnesses_RGB(pix_data, k_bytes, nums_brightnesses);
        else
            count_all_brightnesses_YCbCr(pix_data, k_bytes, char_header, nums_brightnesses);

        if (conversion == 2 || conversion == 3) {
            int min_idx = 0;
            int max_idx = 255;
            while (nums_brightnesses[min_idx] == 0) ++min_idx;
            while (nums_brightnesses[max_idx] == 0) --max_idx;
            offset = min_idx;
            multiplier = 255.0l / (max_idx - min_idx);
            std::cout << "Calculated offset: " << offset << "\n";
            std::cout << "Multiplier: " << std::setprecision(5) << multiplier << "\n";

            if (conversion == 2) {
                change_offset_multiply_all(pix_data, k_bytes, offset, multiplier);
            } else {
                // conversion == 3
                change_offset_multiply_Y(pix_data, k_bytes, offset, multiplier);
            }

        } else {

            // Игнорируем 0.39% пикселей с двух сторон

            int pixels_count = width * height;
            auto ignoring_count = (long long) std::round(0.0039l * pixels_count);

            int min_idx = 0;
            int max_idx = 255;
            // min_idx <= 255 - излишне, но оставим на всякий случай
            while (min_idx <= 255 && (nums_brightnesses[min_idx] == 0 || ignoring_count > 0)) {
                if (nums_brightnesses[min_idx] == 0) {
                    ++min_idx;
                    continue;
                }

                if (ignoring_count >= nums_brightnesses[min_idx]) {
                    ignoring_count -= nums_brightnesses[min_idx];
                    ++min_idx;
                } else {
                    break;
                }
            }

            ignoring_count = (long long) std::round(0.0039l * pixels_count);
            // max_idx >= 0 - излишне, но оставим на всякий случай
            while (max_idx >= 0 && (nums_brightnesses[max_idx] == 0|| ignoring_count > 0)) {
                if (nums_brightnesses[max_idx] == 0) {
                    --max_idx;
                    continue;
                }
                if (ignoring_count >= nums_brightnesses[min_idx]) {
                    ignoring_count -= nums_brightnesses[min_idx];
                    --max_idx;
                } else {
                    break;
                }
            }

            offset = min_idx;
            multiplier = 255.0l / (max_idx - min_idx);
            std::cout << "Calculated offset: " << offset << "\n";
            std::cout << "Multiplier: " << std::setprecision(5) << multiplier << "\n";

            if (conversion == 4) {
                change_offset_multiply_all(pix_data, k_bytes, offset, multiplier);
            } else {
                // conversion == 5
                change_offset_multiply_Y(pix_data, k_bytes, offset, multiplier);
            }
        }

    }

    if (conversion % 2 == 1 && char_header == '6') {
        conversions::YCbCr_601_2_rgb(pix_data, k_bytes);
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

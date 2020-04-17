//
// Created by dudko on 11.03.2020.
//

#include "write_data_pnm.h"
#include <cmath>

void write_header(FILE *file_out, char char_header, int width, int height, unsigned int max_value) {
    fseek(file_out, 0, SEEK_SET);
    fprintf(file_out, "P%c\n%i %i\n%i\n", char_header, width, height, max_value);
}

void write_data(FILE *file_out, int k_bytes, unsigned char *pix_data) {
    fwrite(pix_data, k_bytes, 1, file_out);
}

void free_data(FILE *file_in, FILE *file_out, unsigned char *pix_data) {
    if (file_in) fclose(file_in);
    if (file_out) fclose(file_out);
    if (pix_data) free(pix_data);
}

void free_data(FILE *file, unsigned char *pix_data) {
    free_data(nullptr, file, pix_data);
}

void free_data(FILE *file) {
    free_data(file, nullptr);
}

void free_data(unsigned char *pix_data) {
    free_data(nullptr, pix_data);
}

void write_to_file(FILE *file_out, char char_header, int width, int height, unsigned int max_value,
                   unsigned char *pix_data) {
    write_header(file_out, char_header, width, height, max_value);
    write_data(file_out, width * height, pix_data);
}

double to_sRGB(double _brightness) {
    if (_brightness <= 0.0031308) {
        return 323.0 * _brightness / 25.0;
    } else {
        return (211 * pow(_brightness, 5.0 / 12.0) - 11) / 200.0;
    }
}

double to_sRGB(int brightness) {
    double _brightness = brightness / 255.0;
    return to_sRGB(_brightness);
}

double from_sRGB(double _brightness) {
    if (_brightness <= 0.04045) {
        return 25.0 * _brightness / 323;
    } else {
        return pow((200 * _brightness + 11) / 211.0, 12.0 / 5.0);
    }
}

double from_sRGB(int brightness) {
    double _brightness = brightness / 255.0;
    return from_sRGB(_brightness);
}

double change_pix_gamma_to_print(double _brightness, double gamma) {
    // Гамма коррекция:
    if (gamma > 0) {
        _brightness = std::pow(_brightness, gamma);
    } else {
        _brightness = from_sRGB(_brightness);
    }

    return _brightness;
}

double change_pix_gamma_to_print(unsigned char pix_data, double gamma) {
    double _brightness = pix_data / 255.0;
    return change_pix_gamma_to_print(_brightness, gamma);
}

// ToDO: гамма коррекция
void draw_pix(unsigned char *pix_data, int width, int x, int y, int brightness, double gamma) {
    double _brightness = brightness / 255.0;

    // Гамма коррекция:
    change_pix_gamma_to_print(_brightness, gamma);

    // ToDO: debug
//    double _round_brightness = std::round(255 * _brightness);
//    int _round_brightness_casted_to_int = (int) std::round(255 * _brightness);
//    int _max = std::max((int) std::round((255 * _brightness)), 0);
//    int _min = std::min(std::max((int) std::round((255 * _brightness)), 0), 255);

    pix_data[width * y + x] = std::min(std::max((int) std::round((255 * _brightness)), 0), 255);
}

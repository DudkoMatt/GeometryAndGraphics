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


// ToDO: гамма коррекция
void draw_pix(unsigned char *pix_data, int width, int x, int y, int brightness, double gamma) {
    double _brightness = brightness / 255.0;

    // Гамма коррекция:
    if (gamma > 0) {
        _brightness = std::pow(_brightness, 1.0 / gamma);
    } else {
        // ToDO: sRGB
    }

    pix_data[width * y + x] = std::min(std::max(255 * ((int) std::round(_brightness)), 0), 255);
}
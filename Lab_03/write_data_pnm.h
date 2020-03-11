//
// Created by dudko on 11.03.2020.
//

#ifndef LAB_03_WRITE_DATA_PNM_H
#define LAB_03_WRITE_DATA_PNM_H

#include <iostream>
void write_header(FILE *file_out, char char_header, int width, int height, unsigned int max_value);
void write_data(FILE *file_out, int k_bytes, unsigned char *pix_data);
void free_data(FILE *file_in, FILE *file_out, unsigned char *pix_data);
void free_data(FILE *file, unsigned char *pix_data);
void free_data(FILE *file);
void free_data(unsigned char *pix_data);
void write_to_file(FILE *file_out, char char_header, int width, int height, unsigned int max_value, unsigned char *pix_data);

void draw_pix(unsigned char *pix_data, int width, int x, int y, int brightness, double gamma);

#endif //LAB_03_WRITE_DATA_PNM_H

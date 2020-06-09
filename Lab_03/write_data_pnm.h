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

unsigned char limit_brightness(double brightness);

double to_sRGB(double _brightness);
double to_sRGB(int brightness);
double from_sRGB(double _brightness);
double from_sRGB(int brightness);

double change_pix_gamma_to_print(double _brightness, double gamma);
double change_pix_gamma_to_print(unsigned char pix_data, double gamma);

double change_pix_gamma_from_file(double _brightness, double gamma);
double change_pix_gamma_from_file(unsigned char pix_data, double gamma);

void draw_pix(unsigned char *pix_data, int width, int x, int y, int brightness);
void draw_pix(unsigned char *pix_data, int width, int x, int y, double _brightness);

void decode_gamma_from_file(unsigned char *pix_data, int k, double gamma);

#endif //LAB_03_WRITE_DATA_PNM_H

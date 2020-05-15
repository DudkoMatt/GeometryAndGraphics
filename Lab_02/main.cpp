#include <iostream>
#include <cmath>

void write_header(FILE *file_out, char char_header, int width, int height, unsigned int max_value);

void write_data(FILE *file_out, int k_bytes, unsigned char *pix_data);

void free_data(FILE *file_in, FILE *file_out, unsigned char *pix_data);

void draw_pix(unsigned char *pix_data, int height, int width, int x, int y, double brightness, double gamma, int max_brightness);
double fpart(double x);

// along_y рисует вдоль обеих осей
void wu_line_along_y(unsigned char *pix_data, int width, int height, int brightness, double x0, double y0, double x1, double y1, double gamma);

int main(int argc, char *argv[]) {

    if (argc != 9 && argc != 10) {
        std::cerr << "Wrong number of arguments. Syntax:\n<lab>.exe file_in file_out brightness thickness x0 y0 x1 y1 [gamma]";
        return 1;
    }

    const char *file_in_name = argv[1];
    const char *file_out_name = argv[2];

    double brightness;
    double thickness;
    double x0, y0, x1, y1;

    double gamma = 2.0;

    // Общий случай: 2.0
    // Лучше - 2.2

    try {
        brightness = std::stoi(argv[3]);
        thickness = std::stod(argv[4]);
        x0 = std::stod(argv[5]);
        y0 = std::stod(argv[6]);
        x1 = std::stod(argv[7]);
        y1 = std::stod(argv[8]);
        if (argc == 10)
            gamma = std::stod(argv[9]);
    } catch (std::invalid_argument &e) {
        std::cerr << "Cannot convert one of the numbers from string to int\n";
        return 1;
    }


    // Считывание размеров из файла

    char _char_header;
    int width, height;
    unsigned int max_value;

    FILE* file_in = fopen(file_in_name, "rb");

    if (file_in == nullptr) {
        std::cerr << "Cannot open file to read: " << file_in_name << "\n";
        return 1;
    }

    int scanned = fscanf(file_in, "P%c\n%i %i\n%i\n", &_char_header, &width, &height, &max_value);


    if (width <= 0 || height <= 0 || x0 < 0 || y0 < 0 || x1 < 0 || y1 < 0 || y0 >= height || y1 >= height ||
        x0 >= width || x1 >= width || thickness <= 0 || brightness < 0 || brightness > 255) {
        std::cerr << "Wrong arguments\n";
        fclose(file_in);
        return 1;
    }


    // Перевод из воспринимаемой яркости в фактическую
    brightness = std::pow(brightness, gamma);


    FILE *file_out = nullptr;
    const char char_header = '5';
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

    if (x0 == x1 && y0 == y1) {
        draw_pix(pix_data, height, width, x0, y0, brightness, gamma, brightness);
    } else {
        // Алгоритм Ву со сглаживанием
        wu_line_along_y(pix_data, width, height, brightness, x0, y0, x1, y1, gamma);
    }

    write_header(file_out, char_header, width, height, 255);
    write_data(file_out, k_bytes, pix_data);

    std::cout << (int) pix_data[0];

    free_data(file_in, file_out, pix_data);
    return 0;
}

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

void draw_pix(unsigned char *pix_data, int height, int width, int x, int y, double brightness, double gamma, int max_brightness) {
    if (x >= width || x < 0 || y < 0 || y > height) return;
    brightness = std::min(std::max(brightness, 0.0), 1.0);

    // Гамма коррекция:
    if (gamma > 0) {
        brightness = std::pow(max_brightness * brightness, 1.0 / gamma);
    } else {
        // sRGB -- только частичное решение
    }

    pix_data[width * y + x] = std::min(std::max((int) std::round(brightness), 0), 255);
}

double fpart(double x) {
    return x - std::floor(x);
}

void wu_line_along_y(unsigned char *pix_data, int width, int height, int brightness, double x0, double y0, double x1, double y1, double gamma) {
    // Swap if necessary
    if (x1 < x0) {
        double t = x0;
        x0 = x1;
        x1 = t;

        t = y0;
        y0 = y1;
        y1 = t;
    }

    double dx = x1 - x0;
    double dy = y1 - y0;

    double gradient = dy / dx;     // Угловой коэффициент прямой

    if (dy > dx) {
        gradient = dx / dy;

        double t = x0;
        x0 = y0;
        y0 = t;

        t = x1;
        x1 = y1;
        y1 = t;
    }


    // Обработка начальной точки
    double xend = round(x0);
    double yend = y0 + gradient * (xend - x0);
    double xgap = 1 - fpart(x0);
    double xpxl1 = xend;
    double ypxl1 = std::floor(yend);

    // Отрисовка вдоль оси oY
    if (dy > dx) {

        draw_pix(pix_data, height, width, ypxl1, xpxl1, (1 - fpart(yend)) * xgap, gamma, brightness);
        draw_pix(pix_data, height, width, ypxl1 + 1, xpxl1, (fpart(yend)) * xgap, gamma, brightness);
    } else {
        draw_pix(pix_data, height, width, xpxl1, ypxl1, (1 - fpart(yend)) * xgap, gamma, brightness);
        draw_pix(pix_data, height, width, xpxl1, ypxl1 + 1, (fpart(yend)) * xgap, gamma, brightness);
    }

    double intery = yend + gradient;


    // Обработка конечной точки
    xend = round(x1);
    yend = y1 + gradient * (xend - x1);
    xgap = 1 - fpart(x1);
    double xpxl2 = xend;
    double ypxl2 = std::floor(yend);

    if (dy > dx) {
        draw_pix(pix_data, height, width,  ypxl2, xpxl2, (1 - fpart(yend)) * xgap, gamma, brightness);
        draw_pix(pix_data, height, width,  ypxl2 + 1, xpxl2, (fpart(yend)) * xgap, gamma, brightness);
    } else {

        draw_pix(pix_data, height, width, xpxl2, ypxl2, (1 - fpart(yend)) * xgap, gamma, brightness);
        draw_pix(pix_data, height, width, xpxl2, ypxl2 + 1, (fpart(yend)) * xgap, gamma, brightness);
    }

    for (int x = xpxl1 + 1; x < xpxl2; ++x) {
        if (dy > dx) {
            draw_pix(pix_data, height, width,  std::floor(intery), x, (1 - fpart(intery)), gamma, brightness);
            draw_pix(pix_data, height, width,  std::floor(intery) + 1, x,  fpart(intery), gamma, brightness);
        } else {
            draw_pix(pix_data, height, width, x, std::floor(intery),  (1 - fpart(intery)), gamma, brightness);
            draw_pix(pix_data, height, width, x, std::floor(intery) + 1,  fpart(intery), gamma, brightness);
        }
        intery += gradient;
    }

}

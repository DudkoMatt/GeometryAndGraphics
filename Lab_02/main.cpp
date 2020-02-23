#include <iostream>

void write_header(FILE *file_out, char char_header, int width, int height, unsigned int max_value);

void write_data(FILE *file_out, int k_bytes, unsigned char *pix_data);

void line_along_x(unsigned char *pix_data, int width, int x0, int y0, int x1, int y1);
void line_along_y(unsigned char *pix_data, int height, int width, int x0, int y0, int x1, int y1);

void free_data(FILE *file_out, unsigned char *pix_data);


/*
 *
 * В данной лабороторной работе используется алгоритм Брезенхема для отрисовки линии
 *
 * ToDO: сейчас часть частичного решения
 *
 *
 */


int main(int argc, char *argv[]) {

    if (argc != 8) {
        std::cout << "Wrong number of arguments. Syntax:\n<lab>.exe file_out width height x0 y0 x1 y1";
        return 0;
    }

    const char *file_out_name = argv[1];

    int width, height;
    int x0, y0, x1, y1;

    try {
        width = std::stoi(argv[2]);
        height = std::stoi(argv[3]);
        x0 = std::stoi(argv[4]);
        y0 = std::stoi(argv[5]);
        x1 = std::stoi(argv[6]);
        y1 = std::stoi(argv[7]);
    } catch (std::invalid_argument &e) {
        std::cout << "Cannot convert one of the numbers from string to int\n";
        return 0;
    }

    if (width <= 0 || height <= 0 || x0 < 0 || y0 < 0 || x1 < 0 || y1 < 0 || y0 >= height || y1 >= height ||
        x0 >= width || x1 >= width) {
        std::cout << "Wrong arguments\n";
        return 0;
    }


    // Преобразование координат:
    // Начало координат в левом нижнем углу; вверх - ось y; вправо - ось x

    y1 = height - y1 - 1;
    y0 = height - y0 - 1;

    FILE *file_out = nullptr;
    const char char_header = '5';
    int k_bytes = height * width;

    auto pix_data = (unsigned char *) calloc(k_bytes, 1);
    if (!pix_data) {
        std::cout << "Cannot allocate " << k_bytes << " bytes of memory\n";
        free_data(file_out, pix_data);
        return 0;
    }

    // Initialization
    for (int i = 0; i < k_bytes; ++i) {
        pix_data[i] = 255;
    }

    file_out = fopen(file_out_name, "wb");
    if (file_out == nullptr) {
        std::cout << "Cannot open file to write: " << file_out_name << "\n";
        free_data(file_out, pix_data);
        return 0;
    }

    // Определяем, в каком секторе лежит конечная точка, относительно начальной точки
    // В зависимости от этого вызываем модификацию функции
    int tmp_x = x1 - x0;
    int tmp_y = y0 - y1;

    if (tmp_y <= tmp_x && tmp_y >= -tmp_x) {
        // Справа
        // Построение только в границах: x > 0; y <= x; y >= -x
        line_along_x(pix_data, width, x0, y0, x1, y1);
    } else if (tmp_y <= tmp_x && tmp_y <= -tmp_x) {
        // Снизу
        // Построение только в границах: y < 0; y <= x; y <= -x
        line_along_y(pix_data, height, width, x0, y0, x1, y1);
    } else if (tmp_y >= tmp_x && tmp_y <= -tmp_x) {
        // Слева
        // Построение только в границах: x < 0; y >= x; y <= -x
        line_along_x(pix_data, width, x1, y1, x0, y0);
    } else {
        // Сверху
        // Построение только в границах: y > 0; y >= x; y >= -x
        line_along_y(pix_data, height, width, x1, y1, x0, y0);
    }

    // Отрисовка начальных точек
    pix_data[y0 * width + x0] = 0;
    pix_data[y1 * width + x1] = 0;

    write_header(file_out, char_header, width, height, 255);
    write_data(file_out, k_bytes, pix_data);

    free_data(file_out, pix_data);
    return 0;
}

void write_header(FILE *file_out, char char_header, int width, int height, unsigned int max_value) {
    fseek(file_out, 0, SEEK_SET);
    fprintf(file_out, "P%c\n%i %i\n%i\n", char_header, width, height, max_value);
}

void write_data(FILE *file_out, int k_bytes, unsigned char *pix_data) {
    fwrite(pix_data, k_bytes, 1, file_out);
}

void free_data(FILE *file_out, unsigned char *pix_data) {
    if (file_out) fclose(file_out);
    if (pix_data) free(pix_data);
}

// Ограничение: построение только в границах: x > 0; y <= x; y >= -x
// Алгоритм Брезенхема
void line_along_x(unsigned char *pix_data, int width, int x0, int y0, int x1, int y1) {
    int delta_x = std::abs(x1 - x0);
    int delta_y = std::abs(y1 - y0);
    int error = 0;
    int delta_err = delta_y + 1;
    int y = y0;
    int dir_y = y1 - y0;
    if (dir_y > 0) {
        dir_y = 1;
    }
    if (dir_y < 0) {
        dir_y = -1;
    }
    for (int x = x0; x < x1; ++x) {
        pix_data[y * width + x] = 0;
        error += delta_err;
        if (error >= delta_x + 1) {
            y += dir_y;
            error -= delta_x + 1;
        }
    }
}

// Ограничение: построение только в границах: y < 0; y <= x; y <= -x
void line_along_y(unsigned char *pix_data, int height, int width, int x0, int y0, int x1, int y1) {
    int delta_x = std::abs(x1 - x0);
    int delta_y = std::abs(y1 - y0);
    int error = 0;
    int delta_err = delta_x + 1;
    int x = x0;
    int dir_x = x1 - x0;
    if (dir_x > 0) {
        dir_x = 1;
    }
    if (dir_x < 0) {
        dir_x = -1;
    }
    for (int y = y0; y < y1; ++y) {
        pix_data[y * width + x] = 0;
        error += delta_err;
        if (error >= delta_y + 1) {
            x += dir_x;
            error -= delta_y + 1;
        }
    }
}

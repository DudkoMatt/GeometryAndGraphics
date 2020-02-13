#include <iostream>

void write_header(FILE *file_out, const char *char_header, int width, int height, unsigned int max_value);
void write_data(FILE *file_out, int k_bytes, unsigned char *pix_data);

void inversion(FILE *file_out, const char *char_header, int width, int height, unsigned int max_value, int k_bytes, unsigned char *pix_data);
void vertical_reflection(FILE *file_out, const char *char_header, int width, int height, unsigned int max_value, int k_bytes, const unsigned char *pix_data, bool is_color);
void horizontal_reflection(FILE *file_out, const char *char_header, int width, int height, unsigned int max_value, int k_bytes, const unsigned char *pix_data, bool is_color);
void rotate_clockwise_90(FILE *file_out, const char *char_header, int width, int height, unsigned int max_value, int k_bytes, const unsigned char *pix_data, bool is_color);
void rotate_counter_clockwise_90(FILE *file_out, const char *char_header, int width, int height, unsigned int max_value, int k_bytes, const unsigned char *pix_data, bool is_color);

int main(int argc, char *argv[]) {

    // Часть 1: обработка агрументов командной строки

    if (argc != 4) {
        std::cout << "Wrong number of arguments. Syntax:\n<lab>.exe file_in file_out conversion\n";
        return 0;
    }

    const char *file_in_name = argv[1];
    const char *file_out_name = argv[2];
    int conversion = (int) (*argv[3] - '0');

    if (!(0 <= conversion && conversion <= 4)) {
        std::cout << "Wrong conversion parameter: " << conversion << std::endl;
        return 0;
    }

    // Часть 2: проверка на существование файла. Попытка открыть файл на запись
    // Note: предполагаем, что в картинках нет комментариев

    FILE *file_in;
    file_in = fopen(file_in_name, "rb");

    if (file_in == nullptr) {
        std::cout << "Cannot open file to read: " << file_in_name << "\n";
        return 0;
    }

    // Часть 3: чтение файла

    // Чтение заголовка
    char *char_header = new char;
    int width, height;
    unsigned int max_value;

    int scanned = fscanf(file_in, "P%c\n%i %i\n%i\n", char_header, &width, &height, &max_value);

    if (scanned <= 0) {
        std::cout << "Something went wrong. Error during scanning header\n";
        return 0;
    }

    if (width <= 0 || height <= 0 || max_value <= 0 || max_value > 255) {
        std::cout << "Something went wrong. At least one of the numbers is wrong. Maybe there is a comment in a file\n";
        return 0;
    }

    int k_bytes = height * width;

    bool is_color = false;
    if (*char_header == '6') {
        k_bytes *= 3;
        is_color = true;
    } else if (*char_header != '5') {
        std::cout << "Unsupported file format\n";
        return 0;
    }

    auto pix_data = (unsigned char *) calloc(k_bytes, 1);
    if (!pix_data) {
        std::cout << "Cannot allocate " << k_bytes << " bytes of memory\n";
        return 0;
    }

    int bytes_read = fread(pix_data, 1, k_bytes, file_in);

    if (bytes_read < k_bytes) {
        std::cout << "Can't read all data:\n";
        std::cout << "Expected " << k_bytes << " bytes, but only " << bytes_read << " were read\n";
        return 0;
    }

    long current_pos_in_file = ftell(file_in);
    fseek(file_in, 0, SEEK_END);
    if (current_pos_in_file != ftell(file_in)) {
        std::cout << "Error: file contains more data than expected\n";
        return 0;
    }

    fclose(file_in);

    // Часть 4: обработка действий

    FILE *file_out = fopen(file_out_name, "wb");

    if (file_out == nullptr) {
        std::cout << "Cannot open file to write: " << file_out_name << "\n";
        return 0;
    }

    if (conversion == 0) {
        // Инверсия
        inversion(file_out, char_header, width, height, max_value, k_bytes, pix_data);
    } else if (conversion == 1) {
        // Зеркальное отображение по горизонтали
        horizontal_reflection(file_out, char_header, width, height, max_value, k_bytes, pix_data, is_color);
    } else if (conversion == 2) {
        // Зеркальное отражение по вертикали
        vertical_reflection(file_out, char_header, width, height, max_value, k_bytes, pix_data, is_color);
    } else if (conversion == 3) {
        // Поворот на 90 градусов по часовой стрелке
        rotate_clockwise_90(file_out, char_header, width, height, max_value, k_bytes, pix_data, is_color);
    } else if (conversion == 4) {
        // Поворот на 90 градусов против часовой стрелки
        rotate_counter_clockwise_90(file_out, char_header, width, height, max_value, k_bytes, pix_data, is_color);
    }

    fclose(file_out);
    free(pix_data);
    return 0;
}

void write_header(FILE *file_out, const char *char_header, int width, int height, unsigned int max_value) {
    fseek(file_out, 0, SEEK_SET);
    fprintf(file_out, "P%c\n%i %i\n%i\n", *char_header, width, height, max_value);
}

void write_data(FILE *file_out, int k_bytes, unsigned char *pix_data) {
    fwrite(pix_data, k_bytes, 1, file_out);
}

void inversion(FILE *file_out, const char *char_header, int width, int height, unsigned int max_value, int k_bytes, unsigned char *pix_data) {
    write_header(file_out, char_header, width, height, max_value);
    for (int i = 0; i < k_bytes; ++i) {
        pix_data[i] = max_value - pix_data[i];
    }
    write_data(file_out, k_bytes, pix_data);
}

void vertical_reflection(FILE *file_out, const char *char_header, int width, int height, unsigned int max_value, int k_bytes, const unsigned char *pix_data, bool is_color) {
    write_header(file_out, char_header, width, height, max_value);

    auto new_pix_data = (unsigned char *) calloc(k_bytes, 1);
    if (!new_pix_data) {
        std::cout << "Cannot allocate " << k_bytes << " bytes of memory\n";
        exit(0);
    }

    if (is_color) {
        width *= 3;
    }

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            new_pix_data[(height - i - 1) * width + j] = pix_data[i * width + j];
        }
    }

    write_data(file_out, k_bytes, new_pix_data);
    free(new_pix_data);
}

void horizontal_reflection(FILE *file_out, const char *char_header, int width, int height, unsigned int max_value, int k_bytes, const unsigned char *pix_data, bool is_color) {
    write_header(file_out, char_header, width, height, max_value);

    auto new_pix_data = (unsigned char *) calloc(k_bytes, 1);
    if (!new_pix_data) {
        std::cout << "Cannot allocate " << k_bytes << " bytes of memory\n";
        exit(0);
    }

    if (is_color) {
        width *= 3;
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; j += 3) {
                new_pix_data[i * width + j] = pix_data[i * width + (width - j - 3)];
                new_pix_data[i * width + j + 1] = pix_data[i * width + (width - j - 2)];
                new_pix_data[i * width + j + 2] = pix_data[i * width + (width - j - 1)];
            }
        }
    } else {
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                new_pix_data[i * width + j] = pix_data[i * width + width - j - 1];
            }
        }
    }

    write_data(file_out, k_bytes, new_pix_data);
    free(new_pix_data);
}

void rotate_counter_clockwise_90(FILE *file_out, const char *char_header, int width, int height, unsigned int max_value, int k_bytes, const unsigned char *pix_data, bool is_color){
    // Swap height and width
    write_header(file_out, char_header, height, width, max_value);

    auto new_pix_data = (unsigned char *) calloc(k_bytes, 1);
    if (!new_pix_data) {
        std::cout << "Cannot allocate " << k_bytes << " bytes of memory\n";
        exit(0);
    }

    if (is_color) {
        width *= 3;

        int counter = 0;
        for (int j = width - 3; j >= 0; j -= 3) {
            for (int i = 0; i < height; ++i) {
                new_pix_data[counter++] = pix_data[i * width + j];
                new_pix_data[counter++] = pix_data[i * width + j + 1];
                new_pix_data[counter++] = pix_data[i * width + j + 2];
            }
        }

    } else {
        int counter = 0;
        for (int j = width; j >= 0; --j) {
            for (int i = 0; i < height; ++i) {
                new_pix_data[counter++] = pix_data[i * width + j];
            }
        }
    }

    write_data(file_out, k_bytes, new_pix_data);
    free(new_pix_data);
}

void rotate_clockwise_90(FILE *file_out, const char *char_header, int width, int height, unsigned int max_value, int k_bytes, const unsigned char *pix_data, bool is_color){
    // Swap height and width
    write_header(file_out, char_header, height, width, max_value);

    auto new_pix_data = (unsigned char *) calloc(k_bytes, 1);
    if (!new_pix_data) {
        std::cout << "Cannot allocate " << k_bytes << " bytes of memory\n";
        exit(0);
    }

    if (is_color) {
        width *= 3;

        int counter = 0;
        for (int j = 0; j < width; j += 3) {
            for (int i = height - 1; i >= 0; --i) {
                new_pix_data[counter++] = pix_data[i * width + j];
                new_pix_data[counter++] = pix_data[i * width + j + 1];
                new_pix_data[counter++] = pix_data[i * width + j + 2];
            }
        }

    } else {
        int counter = 0;
        for (int j = 0; j < width; ++j) {
            for (int i = height - 1; i >= 0; --i) {
                new_pix_data[counter++] = pix_data[i * width + j];
            }
        }
    }

    write_data(file_out, k_bytes, new_pix_data);
    free(new_pix_data);
}
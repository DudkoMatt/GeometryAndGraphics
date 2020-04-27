#include <iostream>
#include <cstring>
#include <string>
#include <algorithm>
#include <cmath>
#include "write_data_pnm_v2.h"

bool check_color_space(char *color_space) {
    if (color_space == nullptr)
        return false;
    return strcmp(color_space, "RGB") == 0 ||
    strcmp(color_space, "HSL") == 0 ||
    strcmp(color_space, "HSV") == 0 ||
    strcmp(color_space, "YCbCr.601") == 0 ||
    strcmp(color_space, "YCbCr.709") == 0 ||
    strcmp(color_space, "YCoCg") == 0 ||
    strcmp(color_space, "CMY") == 0;
}

void add_channel(unsigned char *pix_data, int k_channel, int k_bytes_channel, const unsigned char *channel) {
    for (int i = 0; i < k_bytes_channel; ++i) {
        *(pix_data + i * 3 + k_channel) = *(channel + i);
    }
}

void get_separate_channel(const unsigned char *pix_data, int all_bytes, int k_channel, unsigned char *channel_data) {
    for (int i = 0; i < all_bytes / 3; ++i) {
        *(channel_data + i) = *(pix_data + 3 * i + k_channel);
    }
}

void rgb_2_hsv(unsigned char *pix_data, int all_bytes) {
    for (int i = 0; i < all_bytes; i += 3) {
        double R = pix_data[i]     / 255.0;
        double G = pix_data[i + 1] / 255.0;
        double B = pix_data[i + 2] / 255.0;

        double MAX = std::max(R, std::max(G, B));
        double MIN = std::min(R, std::min(G, B));

        double V = MAX;
        double C = MAX - MIN;

        // All in [0.0 .. 1.0]
        //   H in [0 .. 360]
        double H;

        // Calculating Hue
        if (C == 0) {
            H = 0;
        }  else if (V == R) {
            H = (G - B) / C;
        } else if (V == G) {
            H = 2 + (B - R) / C;
        } else {
            // V == B
            H = 4 + (R - G) / C;
        }
        H *= 60;

        if (H < 0)
            H += 360;

        double S_v;
        S_v = V == 0 ? 0 : C / V;

        // Transform to PC range: [0 .. 255]
        // H:
        *(pix_data + i)     = (unsigned char) limit_brightness(std::round(H / 360.0 * 255));
        // S:
        *(pix_data + i + 1) = (unsigned char) limit_brightness(std::round(S_v * 255));
        // V:
        *(pix_data + i + 2) = (unsigned char) limit_brightness(std::round(V * 255));
    }
}

void rgb_2_hsl(unsigned char *pix_data, int all_bytes) {
    for (int i = 0; i < all_bytes; i += 3) {
        double R = pix_data[i]     / 255.0;
        double G = pix_data[i + 1] / 255.0;
        double B = pix_data[i + 2] / 255.0;

        double MAX = std::max(R, std::max(G, B));
        double MIN = std::min(R, std::min(G, B));

        double V = MAX;
        double C = MAX - MIN;
        double L = (MAX + MIN) / 2.0;

        // All in [0.0 .. 1.0]
        //   H in [0 .. 360]
        double H;

        // Calculating Hue
        if (C == 0) {
            H = 0;
        }  else if (V == R) {
            H = (G - B) / C;
        } else if (V == G) {
            H = 2 + (B - R) / C;
        } else {
            // V == B
            H = 4 + (R - G) / C;
        }
        H *= 60;

        if (H < 0)
            H += 360;

        double S_l;
        S_l = L == 0 || L == 1 ? 0 : (V - L) / std::min(L, 1 - L);

        // Transform to PC range: [0 .. 255]
        // H:
        *(pix_data + i)     = (unsigned char) limit_brightness(std::round((H / 360.0 * 255)));
        // S:
        *(pix_data + i + 1) = (unsigned char) limit_brightness(std::round((S_l * 255)));
        // V:
        *(pix_data + i + 2) = (unsigned char) limit_brightness(std::round((L * 255)));
    }
}

template <class T>
bool in_range(T left, T x, T right) {
    return (left <= x) && (x <= right);
}

void hsv_2_rgb(unsigned char *pix_data, int all_bytes) {
    for (int i = 0; i < all_bytes; i += 3) {
        double H = pix_data[i] / 255.0 * 360;
        double S_v = pix_data[i + 1] / 255.0;
        double V = pix_data[i + 2] / 255.0;

        double C = V * S_v;
        double _H = H / 60;
        double X = C * (1 - std::abs(((int) _H) % 2 - 1));

        double _R, _G, _B;

        if (in_range(0., H, 1.)) {
            _R = C;
            _G = X;
            _B = 0;
        } else if (in_range(1., H, 2.)) {
            _R = X;
            _G = C;
            _B = 0;
        } else if (in_range(2., H, 3.)) {
            _R = 0;
            _G = C;
            _B = X;
        } else if (in_range(3., H, 4.)) {
            _R = 0;
            _G = X;
            _B = C;
        } else if (in_range(4., H, 5.)) {
            _R = X;
            _G = 0;
            _B = C;
        } else {
            _R = C;
            _G = 0;
            _B = X;
        }

        double m = V - C;

        // To RGB:
        // R:
        *(pix_data + i)     = (unsigned char) limit_brightness(std::round((_R + m) * 255));
        // G
        *(pix_data + i + 1) = (unsigned char) limit_brightness(std::round((_G + m) * 255));
        // B
        *(pix_data + i + 2) = (unsigned char) limit_brightness(std::round((_B + m) * 255));
    }
}

void hsl_2_rgb(unsigned char *pix_data, int all_bytes) {
    for (int i = 0; i < all_bytes; i += 3) {
        double H = pix_data[i] / 255.0 * 360;
        double S_l = pix_data[i + 1] / 255.0;
        double L = pix_data[i + 2] / 255.0;

        double C = (1 - std::abs(2 * L - 1)) * S_l;
        double _H = H / 60;
        double X = C * (1 - std::abs(((int) _H) % 2 - 1));

        double _R, _G, _B;

        if (std::ceil(_H) == 1) {
            _R = C;
            _G = X;
            _B = 0;
        } else if (std::ceil(_H) == 2) {
            _R = X;
            _G = C;
            _B = 0;
        } else if (std::ceil(_H) == 3) {
            _R = 0;
            _G = C;
            _B = X;
        } else if (std::ceil(_H) == 4) {
            _R = 0;
            _G = X;
            _B = C;
        } else if (std::ceil(_H) == 5) {
            _R = X;
            _G = 0;
            _B = C;
        } else if (std::ceil(_H) == 6) {
            _R = C;
            _G = 0;
            _B = X;
        } else {
            _R = 0;
            _G = 0;
            _B = 0;
        }

        double m = L - C / 2.0;

        // To RGB:
        // R:
        *(pix_data + i)     = (unsigned char) limit_brightness(std::round((_R + m) * 255));
        // G
        *(pix_data + i + 1) = (unsigned char) limit_brightness(std::round((_G + m) * 255));
        // B
        *(pix_data + i + 2) = (unsigned char) limit_brightness(std::round((_B + m) * 255));
    }
}

void rgb_2_YCbCr_601(unsigned char *pix_data, int all_bytes) {
    double K_b = 0.299;
    double K_r = 0.587;
    double K_g = 0.114;

    for (int i = 0; i < all_bytes; i += 3) {
        double R = pix_data[i] / 255.0;
        double G = pix_data[i + 1] / 255.0;
        double B = pix_data[i + 2] / 255.0;

        double Y =   K_r * R + K_g * G + K_b * B;
        double C_b = (B - Y) / (2 * (1 - K_b));
        double C_r = (R - Y) / (2 * (1 - K_r));

        // To YCbCr_601:
        *(pix_data + i)     = (unsigned char) limit_brightness(std::round(Y * 255));
        *(pix_data + i + 1) = (unsigned char) limit_brightness(std::round((C_b + 0.5) * 255));
        *(pix_data + i + 2) = (unsigned char) limit_brightness(std::round((C_r + 0.5) * 255));
    }
}

void YCbCr_601_2_rgb(unsigned char *pix_data, int all_bytes) {
    double K_b = 0.299;
    double K_r = 0.587;
    double K_g = 0.114;

    for (int i = 0; i < all_bytes; i += 3) {
        double Y =   pix_data[i]     / 255.0;
        double C_b = pix_data[i + 1] / 255.0 - 0.5;
        double C_r = pix_data[i + 2] / 255.0 - 0.5;

        double R = Y + (2 - 2 * K_r) * C_r;
        double G = Y - K_b * (2 - 2 * K_b) * C_b / K_g - K_r * (2 - 2 * K_r) * C_r / K_g;
        double B = Y + (2 - 2 * K_b) * C_b;

        // To RGB:
        *(pix_data + i)     = (unsigned char) limit_brightness(std::round(R * 255));
        *(pix_data + i + 1) = (unsigned char) limit_brightness(std::round(G * 255));
        *(pix_data + i + 2) = (unsigned char) limit_brightness(std::round(B * 255));
    }
}

void rgb_2_YCbCr_709(unsigned char *pix_data, int all_bytes) {
    double K_b = 0.0722;
    double K_r = 0.2126;
    double K_g = 0.7152;

    for (int i = 0; i < all_bytes; i += 3) {
        double R = pix_data[i] / 255.0;
        double G = pix_data[i + 1] / 255.0;
        double B = pix_data[i + 2] / 255.0;

        double Y =   K_r * R + K_g * G + K_b * B;
        double C_b = (B - Y) / (2 * (1 - K_b));
        double C_r = (R - Y) / (2 * (1 - K_r));

        // To YCbCr_709:
        *(pix_data + i)     = (unsigned char) limit_brightness(std::round(Y * 255));
        *(pix_data + i + 1) = (unsigned char) limit_brightness(std::round((C_b + 0.5) * 255));
        *(pix_data + i + 2) = (unsigned char) limit_brightness(std::round((C_r + 0.5) * 255));
    }
}

void YCbCr_709_2_rgb(unsigned char *pix_data, int all_bytes) {
    double K_b = 0.0722;
    double K_r = 0.2126;
    double K_g = 0.7152;

    for (int i = 0; i < all_bytes; i += 3) {
        double Y =   pix_data[i]     / 255.0;
        double C_b = pix_data[i + 1] / 255.0 - 0.5;
        double C_r = pix_data[i + 2] / 255.0 - 0.5;

        double R = Y + (2 - 2 * K_r) * C_r;
        double G = Y - K_b * (2 - 2 * K_b) * C_b / K_g - K_r * (2 - 2 * K_r) * C_r / K_g;
        double B = Y + (2 - 2 * K_b) * C_b;

        // To RGB:
        *(pix_data + i)     = (unsigned char) limit_brightness(std::round(R * 255));
        *(pix_data + i + 1) = (unsigned char) limit_brightness(std::round(G * 255));
        *(pix_data + i + 2) = (unsigned char) limit_brightness(std::round(B * 255));
    }
}

void rgb_2_YCoCg(unsigned char *pix_data, int all_bytes) {
    for (int i = 0; i < all_bytes; i += 3) {
        double R = pix_data[i] / 255.0;
        double G = pix_data[i + 1] / 255.0;
        double B = pix_data[i + 2] / 255.0;

        // Y
        *(pix_data + i)     = (unsigned char) limit_brightness(std::round(((R + B) / 4.0 + G / 2.0) * 255));
        // Co
        *(pix_data + i + 1) = (unsigned char) limit_brightness(std::round(((R - B) / 2.0 + 0.5) * 255));
        // Cg
        *(pix_data + i + 2) = (unsigned char) limit_brightness(std::round((G / 2.0 - (B + R) / 4.0 + 0.5) * 255));
    }
}

void YCoCg_2_rgb(unsigned char *pix_data, int all_bytes) {
    for (int i = 0; i < all_bytes; i += 3) {
        double Y = pix_data[i] / 255.0;
        double Co = pix_data[i + 1] / 255.0 - 0.5;
        double Cg = pix_data[i + 2] / 255.0 - 0.5;

        // To RGB:
        *(pix_data + i)     = (unsigned char) limit_brightness(std::round((Y - Cg + Co) * 255));
        *(pix_data + i + 1) = (unsigned char) limit_brightness(std::round((Y + Cg) * 255));
        *(pix_data + i + 2) = (unsigned char) limit_brightness(std::round((Y - Cg - Co) * 255));
    }
}

void rgb_2_cmy(unsigned char *pix_data, int all_bytes) {
    for (int i = 0; i < all_bytes; ++i) {
        *(pix_data + i) = 255 - *(pix_data + i);
    }
}

void cmy_2_rgb(unsigned char *pix_data, int all_bytes) {
    rgb_2_cmy(pix_data, all_bytes);
}

int main(int argc, char *argv[]) {

    // Часть 1: разбор аргументов командной строки

    if (argc != 11) {
        std::cerr << "Wrong number of arguments. Syntax:\n<lab4>.exe -f <from_color_space> -t <to_color_space> -i <count> <input_file_name> -o <count> <output_file_name>\n";
        return 1;
    }

    char _f = 0, _t = 0, _i = 0, _o = 0;
    char *from_color_space = nullptr, *to_color_space = nullptr;
    char *file_in_name = nullptr, *file_out_name = nullptr;
    int i_count = 0, o_count = 0;

    for (int i = 1; i < argc; ++i) {
        if (strlen(argv[i]) == 2) {
            if (strncmp(argv[i], "-f", 2) == 0) {
                _f = 1;
                from_color_space = argv[++i];
            } else if (strncmp(argv[i], "-t", 2) == 0) {
                _t = 1;
                to_color_space = argv[++i];
            } else if (strncmp(argv[i], "-i", 2) == 0) {
                _i = 1;
                i_count = std::stoi(argv[++i]);
                file_in_name = argv[++i];
            } else if (strncmp(argv[i], "-o", 2) == 0) {
                _o = 1;
                o_count = std::stoi(argv[++i]);
                file_out_name = argv[++i];
            } else {
                fprintf(stderr, "Invalid argument: %s\n", argv[i]);
                return 1;
            }
        } else {
            fprintf(stderr, "Invalid argument: %s\n", argv[i]);
            return 1;
        }
    }
    
    if (_f + _t + _i + _o < 4 || (i_count != 1 && i_count != 3) || (o_count != 1 && o_count != 3)
            || !check_color_space(from_color_space) || !check_color_space(to_color_space)
            || file_in_name == nullptr || file_out_name == nullptr) {
        std::cerr << "Wrong arguments\n";
        return 1;
    }

    // Часть 2: проверка на существование файла и чтение файла на ввод

    unsigned char *pix_data = nullptr;
    FILE *file_in = nullptr, *file_out = nullptr;

    char char_header = 0;
    int width = 0, height = 0;
    unsigned int max_value = 0;
    int k_bytes = 0;

    if (i_count == 1) {

        file_in = fopen(file_in_name, "rb");
        if (file_in == nullptr) {
            std::cerr << "Cannot open file to read: " << file_in_name << "\n";
            return 1;
        }

        read_header(char_header, width, height, max_value, file_in);
        if (char_header != '6') {
            std::cerr << "Expected PPM file format\n";
            free_data(file_in, file_out, pix_data);
            return 1;
        }

        k_bytes = 3 * width * height;

        pix_data = (unsigned char *) calloc(k_bytes, 1);
        if (!pix_data) {
            std::cerr << "Cannot allocate " << k_bytes << " bytes of memory\n";
            free_data(file_in, file_out, pix_data);
            return 1;
        }

        read_data(file_in, k_bytes, pix_data);

        fclose(file_in);
        file_in = nullptr;

    } else {
        unsigned char *tmp_pix_data = nullptr;
        int prev_k_bytes = -1;

        for (int i = 1; i <= 3; ++i) {
            std::string tmp_file_name(file_in_name);
            std::string idx("_");
            idx.append(1, '0' + i);
            tmp_file_name.insert(tmp_file_name.begin() + tmp_file_name.find_last_of('.'), idx.begin(), idx.end());

            file_in = fopen(tmp_file_name.data(), "rb");

            if (file_in == nullptr) {
                std::cerr << "Cannot open file to read: " << tmp_file_name << "\n";
                return 1;
            }

            read_header(char_header, width, height, max_value, file_in);
            if (char_header != '5') {
                std::cerr << "Expected PGM file format\n";
                free_data(file_in, file_out, pix_data);
                return 1;
            }

            k_bytes = width * height;

            if (k_bytes != prev_k_bytes && prev_k_bytes != -1) {
                std::cerr << "Different amount of data in files\n";
                free_data(file_in, file_out, pix_data);
                if (tmp_pix_data)
                    free(tmp_pix_data);
                return 1;
            } else {
                prev_k_bytes = k_bytes;
            }
            
            if (i == 1) {
                pix_data = (unsigned char *) calloc(3 * k_bytes, 1);
                if (!pix_data) {
                    std::cerr << "Cannot allocate " << 3 * k_bytes << " bytes of memory\n";
                    free_data(file_in, file_out, pix_data);
                    return 1;
                }
                
                // Initialization
                for (int __i = 0; __i < k_bytes; ++__i) {
                    pix_data[__i] = 0;
                }

                tmp_pix_data = (unsigned char *) calloc(k_bytes, 1);
                if (!tmp_pix_data) {
                    std::cerr << "Cannot allocate " << k_bytes << " bytes of memory\n";
                    free_data(file_in, file_out, pix_data);
                    return 1;
                }
            }

            for (int __i = 0; __i < k_bytes; ++__i) {
                *(tmp_pix_data + i) = 0;
            }

            read_data(file_in, k_bytes, tmp_pix_data);
            add_channel(pix_data, i - 1, k_bytes, tmp_pix_data);

            fclose(file_in);
            file_in = nullptr;
        }

        if (tmp_pix_data)
            free(tmp_pix_data);
        k_bytes *= 3;
    }


    // Часть 3: преобразования

    // Conversions:
    // RGB <=> any, except HSL
    // HSV <=> HSL

    if (strcmp(from_color_space, to_color_space) != 0) {

        // Изначально преобразуем в RGB
        
        if (strcmp(from_color_space, "HSL") == 0) {
            hsl_2_rgb(pix_data, k_bytes);
        } else if (strcmp(from_color_space, "HSV") == 0) {
            hsv_2_rgb(pix_data, k_bytes);
        } else if (strcmp(from_color_space, "YCbCr.601") == 0) {
            YCbCr_601_2_rgb(pix_data, k_bytes);
        } else if (strcmp(from_color_space, "YCbCr.709") == 0) {
            YCbCr_709_2_rgb(pix_data, k_bytes);
        } else if (strcmp(from_color_space, "YCoCg") == 0) {
            YCoCg_2_rgb(pix_data, k_bytes);
        } else if (strcmp(from_color_space, "CMY") == 0) {
            cmy_2_rgb(pix_data, k_bytes);
        }

        // Затем в нужное пространство

        if (strcmp(to_color_space, "HSL") == 0) {
            rgb_2_hsl(pix_data, k_bytes);
        } else if (strcmp(to_color_space, "HSV") == 0) {
            rgb_2_hsv(pix_data, k_bytes);
        } else if (strcmp(to_color_space, "YCbCr.601") == 0) {
            rgb_2_YCbCr_601(pix_data, k_bytes);
        } else if (strcmp(to_color_space, "YCbCr.709") == 0) {
            rgb_2_YCbCr_709(pix_data, k_bytes);
        } else if (strcmp(to_color_space, "YCoCg") == 0) {
            rgb_2_YCoCg(pix_data, k_bytes);
        } else if (strcmp(to_color_space, "CMY") == 0) {
            rgb_2_cmy(pix_data, k_bytes);
        }
    }

    // Часть 4: вывод в файл(-ы)

    if (o_count == 1) {

        file_out = fopen(file_out_name, "wb");
        if (file_out == nullptr) {
            std::cerr << "Cannot open file to write: " << file_out_name << "\n";
            free_data(file_in, file_out, pix_data);
            return 1;
        }

        color::write_to_file(file_out, width, height, max_value, pix_data);
        fclose(file_out);

    } else {
        unsigned char *tmp_channel_data = nullptr;

        for (int i = 1; i <= 3; ++i) {
            std::string tmp_file_name(file_out_name);
            std::string idx("_");
            idx.append(1, '0' + i);
            tmp_file_name.insert(tmp_file_name.begin() + tmp_file_name.find_last_of('.'), idx.begin(), idx.end());

            file_out = fopen(tmp_file_name.data(), "wb");
            if (file_out == nullptr) {
                std::cerr << "Cannot open file to write: " << tmp_file_name << "\n";
                free_data(file_in, file_out, pix_data);
                return 1;
            }

            if (tmp_channel_data == nullptr) {
                tmp_channel_data = (unsigned char *) calloc(k_bytes / 3, 1);
                if (tmp_channel_data == nullptr) {
                    std::cerr << "Cannot allocate " << k_bytes / 3 << " bytes of memory\n";
                    free_data(file_in, file_out, pix_data);
                    return 1;
                }
            }

            for (int j = 0; j < k_bytes / 3; ++j) {
                *(tmp_channel_data + i) = 0;
            }

            get_separate_channel(pix_data, k_bytes, i - 1, tmp_channel_data);
            gray::write_to_file(file_out, width, height, max_value, tmp_channel_data);
            fclose(file_out);
            file_out = nullptr;
        }

        if (tmp_channel_data)
            free(tmp_channel_data);
    }

    free_data(file_in, file_out, pix_data);
    return 0;
}

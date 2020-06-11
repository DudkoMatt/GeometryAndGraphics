//
// Created by dudko on 02.05.2020.
//

#include "conversions_color_spaces.h"
#include "../Lab_04/write_data_pnm_v2.h"
#include <cstring>
#include <cmath>
#include <algorithm>

namespace conversions {
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

    void
    get_separate_channel(const unsigned char *pix_data, int all_bytes, int k_channel, unsigned char *channel_data) {
        for (int i = 0; i < all_bytes / 3; ++i) {
            *(channel_data + i) = *(pix_data + 3 * i + k_channel);
        }
    }

    void rgb_2_hsv(unsigned char *pix_data, int all_bytes) {
        for (int i = 0; i < all_bytes; i += 3) {
            double R = pix_data[i] / 255.0;
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
            } else if (V == R) {
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
            *(pix_data + i) = (unsigned char) limit_brightness(std::round(H / 360.0 * 255));
            // S:
            *(pix_data + i + 1) = (unsigned char) limit_brightness(std::round(S_v * 255));
            // V:
            *(pix_data + i + 2) = (unsigned char) limit_brightness(std::round(V * 255));
        }
    }

    void rgb_2_hsl(unsigned char *pix_data, int all_bytes) {
        for (int i = 0; i < all_bytes; i += 3) {
            double R = pix_data[i] / 255.0;
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
            } else if (V == R) {
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
            *(pix_data + i) = (unsigned char) limit_brightness(std::round((H / 360.0 * 255)));
            // S:
            *(pix_data + i + 1) = (unsigned char) limit_brightness(std::round((S_l * 255)));
            // V:
            *(pix_data + i + 2) = (unsigned char) limit_brightness(std::round((L * 255)));
        }
    }

    template<class T>
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
            double X = C * (1 - std::abs(((int) _H) % 2 + (_H - (int) _H) - 1));

            double _R, _G, _B;

            if (in_range(0.0, _H, 1.0)) {
                _R = C;
                _G = X;
                _B = 0;
            } else if (in_range(1.0, _H, 2.0)) {
                _R = X;
                _G = C;
                _B = 0;
            } else if (in_range(2.0, _H, 3.0)) {
                _R = 0;
                _G = C;
                _B = X;
            } else if (in_range(3.0, _H, 4.0)) {
                _R = 0;
                _G = X;
                _B = C;
            } else if (in_range(4.0, _H, 5.0)) {
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
            *(pix_data + i) = (unsigned char) limit_brightness(std::round((_R + m) * 255));
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
            double X = C * (1 - std::abs(((int) _H) % 2 + (_H - (int) _H) - 1));

            double _R, _G, _B;

            if (std::ceil(_H) - 1.0 < 0.001) {
                _R = C;
                _G = X;
                _B = 0;
            } else if (std::ceil(_H) - 2.0 < 0.001) {
                _R = X;
                _G = C;
                _B = 0;
            } else if (std::ceil(_H) - 3.0 < 0.001) {
                _R = 0;
                _G = C;
                _B = X;
            } else if (std::ceil(_H) - 4.0 < 0.001) {
                _R = 0;
                _G = X;
                _B = C;
            } else if (std::ceil(_H) - 5.0 < 0.001) {
                _R = X;
                _G = 0;
                _B = C;
            } else if (std::ceil(_H) - 6.0 < 0.001) {
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
            *(pix_data + i) = (unsigned char) limit_brightness(std::round((_R + m) * 255));
            // G
            *(pix_data + i + 1) = (unsigned char) limit_brightness(std::round((_G + m) * 255));
            // B
            *(pix_data + i + 2) = (unsigned char) limit_brightness(std::round((_B + m) * 255));
        }
    }

    void rgb_2_YCbCr_601(unsigned char *pix_data, int all_bytes) {
        double K_r = 0.299;
        double K_g = 0.587;
        double K_b = 0.114;

        for (int i = 0; i < all_bytes; i += 3) {
            double R = pix_data[i] / 255.0;
            double G = pix_data[i + 1] / 255.0;
            double B = pix_data[i + 2] / 255.0;

            double Y = K_r * R + K_g * G + K_b * B;
            double C_b = (B - Y) / (2 * (1 - K_b));
            double C_r = (R - Y) / (2 * (1 - K_r));

            // To YCbCr_601:
            *(pix_data + i) = (unsigned char) limit_brightness(std::round(Y * 255));
            *(pix_data + i + 1) = (unsigned char) limit_brightness(std::round((C_b + 0.5) * 255));
            *(pix_data + i + 2) = (unsigned char) limit_brightness(std::round((C_r + 0.5) * 255));
        }
    }

    void YCbCr_601_2_rgb(unsigned char *pix_data, int all_bytes) {
        double K_r = 0.299;
        double K_g = 0.587;
        double K_b = 0.114;

        for (int i = 0; i < all_bytes; i += 3) {
            double Y = pix_data[i] / 255.0;
            double C_b = pix_data[i + 1] / 255.0 - 0.5;
            double C_r = pix_data[i + 2] / 255.0 - 0.5;

            double R = Y + (2 - 2 * K_r) * C_r;
            double G = Y - K_b * (2 - 2 * K_b) * C_b / K_g - K_r * (2 - 2 * K_r) * C_r / K_g;
            double B = Y + (2 - 2 * K_b) * C_b;

            // To RGB:
            *(pix_data + i) = (unsigned char) limit_brightness(std::round(R * 255));
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

            double Y = K_r * R + K_g * G + K_b * B;
            double C_b = (B - Y) / (2 * (1 - K_b));
            double C_r = (R - Y) / (2 * (1 - K_r));

            // To YCbCr_709:
            *(pix_data + i) = (unsigned char) limit_brightness(std::round(Y * 255));
            *(pix_data + i + 1) = (unsigned char) limit_brightness(std::round((C_b + 0.5) * 255));
            *(pix_data + i + 2) = (unsigned char) limit_brightness(std::round((C_r + 0.5) * 255));
        }
    }

    void YCbCr_709_2_rgb(unsigned char *pix_data, int all_bytes) {
        double K_b = 0.0722;
        double K_r = 0.2126;
        double K_g = 0.7152;

        for (int i = 0; i < all_bytes; i += 3) {
            double Y = pix_data[i] / 255.0;
            double C_b = pix_data[i + 1] / 255.0 - 0.5;
            double C_r = pix_data[i + 2] / 255.0 - 0.5;

            double R = Y + (2 - 2 * K_r) * C_r;
            double G = Y - K_b * (2 - 2 * K_b) * C_b / K_g - K_r * (2 - 2 * K_r) * C_r / K_g;
            double B = Y + (2 - 2 * K_b) * C_b;

            // To RGB:
            *(pix_data + i) = (unsigned char) limit_brightness(std::round(R * 255));
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
            *(pix_data + i) = (unsigned char) limit_brightness(std::round(((R + B) / 4.0 + G / 2.0) * 255));
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
            *(pix_data + i) = (unsigned char) limit_brightness(std::round((Y - Cg + Co) * 255));
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
}
//
// Created by dudko on 02.05.2020.
//

#ifndef LAB_05_CONVERSIONS_COLOR_SPACES_H
#define LAB_05_CONVERSIONS_COLOR_SPACES_H

namespace conversions {
    bool check_color_space(char *color_space);

    void add_channel(unsigned char *pix_data, int k_channel, int k_bytes_channel, const unsigned char *channel);

    void
    get_separate_channel(const unsigned char *pix_data, int all_bytes, int k_channel, unsigned char *channel_data);

    void rgb_2_hsv(unsigned char *pix_data, int all_bytes);

    void rgb_2_hsl(unsigned char *pix_data, int all_bytes);

    template<class T>
    bool in_range(T left, T x, T right);

    void hsv_2_rgb(unsigned char *pix_data, int all_bytes);

    void hsl_2_rgb(unsigned char *pix_data, int all_bytes);

    void rgb_2_YCbCr_601(unsigned char *pix_data, int all_bytes);

    void YCbCr_601_2_rgb(unsigned char *pix_data, int all_bytes);

    void rgb_2_YCbCr_709(unsigned char *pix_data, int all_bytes);

    void YCbCr_709_2_rgb(unsigned char *pix_data, int all_bytes);

    void rgb_2_YCoCg(unsigned char *pix_data, int all_bytes);

    void YCoCg_2_rgb(unsigned char *pix_data, int all_bytes);

    void rgb_2_cmy(unsigned char *pix_data, int all_bytes);

    void cmy_2_rgb(unsigned char *pix_data, int all_bytes);
}

#endif //LAB_05_CONVERSIONS_COLOR_SPACES_H

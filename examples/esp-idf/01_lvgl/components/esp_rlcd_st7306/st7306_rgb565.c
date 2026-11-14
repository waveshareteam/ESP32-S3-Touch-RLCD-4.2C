#include "st7306_rgb565.h"

/* Return the nearest of the three RGB colors, using squared RGB distance.
 * Values match the verified panel codes: white=1, red=2, black=3.
 */
static uint8_t rgb565_color(const uint8_t *pixel)
{
    const uint16_t rgb = (uint16_t)pixel[0] | ((uint16_t)pixel[1] << 8);
    int r = (rgb >> 11) & 31;
    int g = (rgb >> 5) & 63;
    int b = rgb & 31;
    r = (r << 3) | (r >> 2);
    g = (g << 2) | (g >> 4);
    b = (b << 3) | (b >> 2);

    const int black_distance = r * r + g * g + b * b;
    const int red_distance = (255 - r) * (255 - r) + g * g + b * b;
    const int white_distance = (255 - r) * (255 - r) +
                               (255 - g) * (255 - g) + (255 - b) * (255 - b);
    if (red_distance < black_distance && red_distance < white_distance) {
        return 2;
    }
    return white_distance < black_distance ? 1 : 3;
}

static uint8_t pixel_bits(const uint8_t *pixel, uint8_t black_mask, bool swap_red_white)
{
    uint8_t color = rgb565_color(pixel);
    if (swap_red_white && color != 3) {
        color = color == 2 ? 1 : 2;
    }
    return color == 3 ? black_mask : color == 2 ? (black_mask & 0xCC) : (black_mask & 0x33);
}

bool st7306_rgb565_pack(const uint8_t *src, size_t src_size, size_t src_stride,
                       uint8_t *dst, size_t dst_size, bool swap_red_white)
{
    const size_t row_bytes = ST7306_RGB565_WIDTH * 2;
    if (!src || !dst || src_stride < row_bytes ||
        src_stride > SIZE_MAX / ST7306_RGB565_HEIGHT ||
        src_size < src_stride * (ST7306_RGB565_HEIGHT - 1) + row_bytes ||
        dst_size < ST7306_RGB565_FRAME_BYTES) {
        return false;
    }

    size_t out = 0;
    for (size_t x = 0; x < ST7306_RGB565_WIDTH; x += 2) {
        for (size_t y = 0; y < ST7306_RGB565_HEIGHT; y += 2) {
            const uint8_t *row0 = src + y * src_stride + x * 2;
            const uint8_t *row1 = row0 + src_stride;
            /* Same four display pixels as fill_cell_r270() in one byte. */
            dst[out++] = pixel_bits(row0,     0xA0, swap_red_white) |
                         pixel_bits(row0 + 2, 0x50, swap_red_white) |
                         pixel_bits(row1,     0x0A, swap_red_white) |
                         pixel_bits(row1 + 2, 0x05, swap_red_white);
        }
    }
    return true;
}

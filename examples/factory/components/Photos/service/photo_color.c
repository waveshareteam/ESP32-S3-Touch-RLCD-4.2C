#include "photo_service.h"
#include <stdlib.h>
#include <string.h>

bool photo_rgb565_to_i2(const uint8_t *rgb, size_t size, unsigned width, unsigned height,
                        uint8_t *out, size_t capacity)
{
    if (!rgb || !out || !width || !height || width > PHOTO_MAX_PIXELS / height ||
        size < (size_t)width * height * 2 || capacity < PHOTO_I2_BYTES) return false;
    int *errors = calloc(2 * (PHOTO_WIDTH + 2), sizeof(int));
    if (!errors) return false;
    int *current = errors, *next = errors + PHOTO_WIDTH + 2;
    static const uint8_t palette[16] = {
        0, 0, 0, 255, 255, 255, 255, 255,
        0, 0, 255, 255, 255, 255, 255, 255,
    };
    /* I2 indexes: black=0, white=1, red=2. Keep aspect-fit margins white. */
    memset(out, 0x55, PHOTO_I2_BYTES);
    memcpy(out, palette, sizeof(palette));
    unsigned dw = PHOTO_WIDTH, dh = (unsigned)((uint64_t)height * dw / width);
    if (dh > PHOTO_HEIGHT) { dh = PHOTO_HEIGHT; dw = (unsigned)((uint64_t)width * dh / height); }
    if (!dw) dw = 1;
    if (!dh) dh = 1;
    unsigned ox = (PHOTO_WIDTH - dw) / 2, oy = (PHOTO_HEIGHT - dh) / 2;
    for (unsigned y = 0; y < dh; ++y) {
        unsigned y0 = y * height / dh, y1 = (y + 1) * height / dh;
        if (y1 <= y0) y1 = y0 + 1;
        for (unsigned x = 0; x < dw; ++x) {
            unsigned x0 = x * width / dw, x1 = (x + 1) * width / dw;
            if (x1 <= x0) x1 = x0 + 1;
            uint64_t sum_r = 0, sum_g = 0, sum_b = 0;
            for (unsigned sy = y0; sy < y1; ++sy) {
                for (unsigned sx = x0; sx < x1; ++sx) {
                    const uint8_t *pixel = rgb + ((size_t)sy * width + sx) * 2;
                    unsigned value = pixel[0] | ((unsigned)pixel[1] << 8);
                    sum_r += (value >> 11) * 255 / 31;
                    sum_g += ((value >> 5) & 63) * 255 / 63;
                    sum_b += (value & 31) * 255 / 31;
                }
            }
            unsigned count = (y1 - y0) * (x1 - x0);
            unsigned r = sum_r / count, g = sum_g / count, b = sum_b / count;
            unsigned index;
            /* Detect red before luminance conversion, including JPEG edge variation. */
            if (r >= 112 && r >= g + 48 && r >= b + 48) {
                index = 2;
            } else {
                int gray = (int)((77 * r + 150 * g + 29 * b + 128) >> 8) + current[x + 1] / 16;
                if (gray < 0) gray = 0;
                if (gray > 255) gray = 255;
                int quantized = gray >= 128 ? 255 : 0;
                index = quantized ? 1 : 0;
                /* Preserve grayscale detail without spreading error through red regions. */
                int error = gray - quantized;
                current[x + 2] += error * 7;
                next[x] += error * 3; next[x + 1] += error * 5; next[x + 2] += error;
            }
            unsigned shift = 6 - 2 * ((x + ox) % 4);
            size_t offset = 16 + (y + oy) * PHOTO_I2_STRIDE + (x + ox) / 4;
            out[offset] = (out[offset] & ~(3U << shift)) | (index << shift);
        }
        int *swap = current; current = next; next = swap;
        memset(next, 0, (PHOTO_WIDTH + 2) * sizeof(int));
    }
    free(errors);
    return true;
}

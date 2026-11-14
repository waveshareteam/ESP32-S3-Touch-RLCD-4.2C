#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ST7306_RGB565_WIDTH       400
#define ST7306_RGB565_HEIGHT      300
#define ST7306_RGB565_FRAME_BYTES 30000

/* Convert a little-endian RGB565 landscape frame to the supplier's native
 * 12-subpixel x 2-row RAM cells. Source/destination must not overlap.
 * Use swap_red_white=false for this panel's verified FF=black, CC=red, 33=white map.
 */
bool st7306_rgb565_pack(const uint8_t *src, size_t src_size, size_t src_stride,
                       uint8_t *dst, size_t dst_size, bool swap_red_white);

#ifdef __cplusplus
}
#endif

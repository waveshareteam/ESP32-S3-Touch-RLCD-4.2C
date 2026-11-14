#pragma once

#include <stddef.h>
#include <stdint.h>

/* Swap the two rows in a native 2x2 cell. Keep the two subpixel bits
 * together so mirroring preserves white=0x33, red=0xCC and black=0xFF.
 */
static inline uint8_t bsp_display_mirror_cell_y(uint8_t value)
{
    return (uint8_t)((value << 4) | (value >> 4));
}

/* Mirror Y within each native column, retaining the original X direction. */
static inline void bsp_display_mirror_native_y(uint8_t *frame, size_t cell_columns,
                                              size_t cell_rows)
{
    for (size_t x = 0; x < cell_columns; ++x) {
        uint8_t *column = frame + x * cell_rows;
        for (size_t y = 0; y < (cell_rows + 1) / 2; ++y) {
            const size_t opposite = cell_rows - 1 - y;
            const uint8_t first = column[y];
            column[y] = bsp_display_mirror_cell_y(column[opposite]);
            column[opposite] = bsp_display_mirror_cell_y(first);
        }
    }
}

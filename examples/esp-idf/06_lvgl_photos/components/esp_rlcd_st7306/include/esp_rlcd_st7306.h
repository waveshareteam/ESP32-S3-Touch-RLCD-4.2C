#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_heap_caps.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ST7306_SWRESET 0x01
#define ST7306_SLPIN   0x10
#define ST7306_SLPOUT  0x11
#define ST7306_INVOFF  0x20
#define ST7306_INVON   0x21
#define ST7306_DISPOFF 0x28
#define ST7306_DISPON  0x29
#define ST7306_CASET   0x2A
#define ST7306_RASET   0x2B
#define ST7306_RAMWR   0x2C
#define ST7306_TEOFF   0x34
#define ST7306_MADCTL  0x36
#define ST7306_IDMOFF  0x38
#define ST7306_IDMON   0x39
#define ST7306_COLMOD  0x3A

#define ST7306_NVMLOAD 0xD6
#define ST7306_BSTEN   0xD1
#define ST7306_GCTRL   0xC0
#define ST7306_VSHP    0xC1
#define ST7306_VSLP    0xC2
#define ST7306_VSHN    0xC4
#define ST7306_VSLN    0xC5
#define ST7306_OSC     0xD8
#define ST7306_DUTY    0xB0
#define ST7306_GATEEQ  0xB2
#define ST7306_SRCSEQ  0xB3
#define ST7306_GATSEQ  0xB4
#define ST7306_FRCTRL  0xB7
#define ST7306_NLINV   0xB9
#define ST7306_INVSET  0xB8
#define ST7306_AUTOPWR 0xD0

/* Logical subpixel codes verified on this panel with OSPTEK HPM init. */
#define ST7306_COLOR_WHITE 0x01
#define ST7306_COLOR_BLACK 0x03
#define ST7306_COLOR_RED   0x02

typedef struct {
    int cmd;
    const void *data;
    size_t data_bytes;
    unsigned int delay_ms;
} st7306_lcd_init_cmd_t;

typedef enum {
    ST7306_PIXEL_MAPPING_PORTRAIT = 0,
    ST7306_PIXEL_MAPPING_LANDSCAPE,
} st7306_pixel_mapping_t;

typedef struct {
    uint16_t hor_res;
    uint16_t ver_res;
    uint8_t col_start;
    uint8_t col_end;
    uint8_t row_start;
    uint8_t row_end;
    const st7306_lcd_init_cmd_t *init_cmds;
    uint16_t init_cmds_size;
} st7306_vendor_config_t;

typedef struct {
    uint16_t width;
    uint16_t height;
    st7306_pixel_mapping_t mapping;
    uint8_t *buffer;
    size_t buffer_size;
    uint16_t *index_lut;
    uint8_t *bit_lut;  /* Both subpixel bits for each display pixel. */
} st7306_frame_buffer_t;

esp_err_t esp_lcd_new_panel_st7306(const esp_lcd_panel_io_handle_t io,
                                   const esp_lcd_panel_dev_config_t *panel_dev_config,
                                   esp_lcd_panel_handle_t *ret_panel);

esp_err_t esp_rlcd_st7306_frame_buffer_init(st7306_frame_buffer_t *fb,
                                            uint16_t width,
                                            uint16_t height,
                                            st7306_pixel_mapping_t mapping,
                                            uint32_t caps);
void esp_rlcd_st7306_frame_buffer_deinit(st7306_frame_buffer_t *fb);
void esp_rlcd_st7306_frame_buffer_clear(st7306_frame_buffer_t *fb, uint8_t color);
esp_err_t esp_rlcd_st7306_frame_buffer_set_pixel(st7306_frame_buffer_t *fb,
                                                 uint16_t x,
                                                 uint16_t y,
                                                 uint8_t color);
esp_err_t esp_rlcd_st7306_frame_buffer_draw_line(st7306_frame_buffer_t *fb,
                                                 int x0,
                                                 int y0,
                                                 int x1,
                                                 int y1,
                                                 uint8_t color);
esp_err_t esp_rlcd_st7306_frame_buffer_draw_rect(st7306_frame_buffer_t *fb,
                                                 int x,
                                                 int y,
                                                 int width,
                                                 int height,
                                                 uint8_t color);
esp_err_t esp_rlcd_st7306_frame_buffer_fill_rect(st7306_frame_buffer_t *fb,
                                                 int x,
                                                 int y,
                                                 int width,
                                                 int height,
                                                 uint8_t color);
esp_err_t esp_rlcd_st7306_frame_buffer_flush(esp_lcd_panel_handle_t panel, const st7306_frame_buffer_t *fb);

#define ST7306_PANEL_BUS_SPI_CONFIG(sclk, mosi, max_trans_sz) \
    {                                                         \
        .sclk_io_num = (sclk),                                \
        .mosi_io_num = (mosi),                                \
        .miso_io_num = -1,                                    \
        .quadwp_io_num = -1,                                  \
        .quadhd_io_num = -1,                                  \
        .max_transfer_sz = (max_trans_sz),                    \
    }

#define ST7306_PANEL_IO_SPI_CONFIG(cs, dc, callback, callback_ctx) \
    {                                                              \
        .cs_gpio_num = (cs),                                       \
        .dc_gpio_num = (dc),                                       \
        .spi_mode = 0,                                             \
        .pclk_hz = 40 * 1000 * 1000,                               \
        .trans_queue_depth = 10,                                   \
        .on_color_trans_done = (callback),                         \
        .user_ctx = (callback_ctx),                                \
        .lcd_cmd_bits = 8,                                         \
        .lcd_param_bits = 8,                                       \
    }

#ifdef __cplusplus
}
#endif

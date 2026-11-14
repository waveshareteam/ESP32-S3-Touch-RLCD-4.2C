#include "rlcd_arduino_config.h"
#include <assert.h>
#include <stdbool.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <sys/cdefs.h>
#include "driver/gpio.h"
#include "esp_check.h"
#include "esp_heap_caps.h"
#include "esp_idf_version.h"
#include "esp_lcd_panel_commands.h"
#include "esp_lcd_panel_interface.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_rlcd_st7306.h"

static const char *TAG = "st7306";

typedef struct {
    esp_lcd_panel_t base;
    esp_lcd_panel_io_handle_t io;
    int reset_gpio_num;
    bool reset_level;
    int x_gap;
    int y_gap;
    uint16_t hor_res;
    uint16_t ver_res;
    uint8_t col_start;
    uint8_t col_end;
    uint8_t row_start;
    uint8_t row_end;
    uint8_t madctl_val;
    uint8_t colmod_val;
    const st7306_lcd_init_cmd_t *init_cmds;
    uint16_t init_cmds_size;
} st7306_panel_t;

static esp_err_t panel_st7306_del(esp_lcd_panel_t *panel);
static esp_err_t panel_st7306_reset(esp_lcd_panel_t *panel);
static esp_err_t panel_st7306_init(esp_lcd_panel_t *panel);
static esp_err_t panel_st7306_draw_bitmap(esp_lcd_panel_t *panel, int x_start, int y_start, int x_end, int y_end, const void *color_data);
static esp_err_t panel_st7306_invert_color(esp_lcd_panel_t *panel, bool invert_color_data);
static esp_err_t panel_st7306_mirror(esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y);
static esp_err_t panel_st7306_swap_xy(esp_lcd_panel_t *panel, bool swap_axes);
static esp_err_t panel_st7306_set_gap(esp_lcd_panel_t *panel, int x_gap, int y_gap);
static esp_err_t panel_st7306_disp_on_off(esp_lcd_panel_t *panel, bool on_off);
static void st7306_get_pixel_location(const st7306_frame_buffer_t *fb, uint16_t x, uint16_t y, uint16_t *index, uint8_t *mask);
static bool st7306_clip_point(const st7306_frame_buffer_t *fb, int x, int y);

/* Adapted from OSPTEK esp32s3-st7306-spi-demo, Initial_ST7306().
 * Source blob: fee3aaa5ddf063f58dcd3f5ffe8ce42d6bded6e5.
 * Keep command order, payloads, HPM and inversion settings unchanged.
 */
static const st7306_lcd_init_cmd_t st7306_default_init_cmds[] = {
    {0xD6, (uint8_t[]){0x17, 0x02}, 2, 0},
    {0xD1, (uint8_t[]){0x01}, 1, 0},
    {0xC0, (uint8_t[]){0x12, 0x0A}, 2, 0},
    {0xC1, (uint8_t[]){0x73, 0x3E, 0x3C, 0x3C}, 4, 0},
    {0xC2, (uint8_t[]){0x00, 0x21, 0x23, 0x23}, 4, 0},
    {0xC4, (uint8_t[]){0x32, 0x5C, 0x5A, 0x5A}, 4, 0},
    {0xC5, (uint8_t[]){0x32, 0x35, 0x37, 0x37}, 4, 0},
    {0xD8, (uint8_t[]){0xA6, 0xE9}, 2, 0},
    {0xB2, (uint8_t[]){0x12}, 1, 0},
    {0xB3, (uint8_t[]){0xE5, 0xF6, 0x17, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x71}, 10, 0},
    {0xB4, (uint8_t[]){0x05, 0x46, 0x77, 0x77, 0x77, 0x77, 0x76, 0x45}, 8, 0},
    {0x62, (uint8_t[]){0x32, 0x03, 0x1F}, 3, 0},
    {0xB7, (uint8_t[]){0x13}, 1, 0},
    {0xB0, (uint8_t[]){0x64}, 1, 0},
    {0x11, NULL, 0, 120},
    {0xC9, (uint8_t[]){0x00}, 1, 0},
    {0x36, (uint8_t[]){0x48}, 1, 0},
    {0x3A, (uint8_t[]){0x11}, 1, 0},
    {0xB9, (uint8_t[]){0x20}, 1, 0},
    {0xB8, (uint8_t[]){0x29}, 1, 0},
    {0x2A, (uint8_t[]){0x05, 0x36}, 2, 0},
    {0x2B, (uint8_t[]){0x00, 0xC7}, 2, 0},
    {0x35, (uint8_t[]){0x00}, 1, 0},
    {0xD0, (uint8_t[]){0xFF}, 1, 0},
    {0x38, NULL, 0, 0},
    {0x29, NULL, 0, 0},
    {0x20, NULL, 0, 0},
    {0xBB, (uint8_t[]){0x4F}, 1, 0},
};

esp_err_t esp_lcd_new_panel_st7306(const esp_lcd_panel_io_handle_t io,
                                   const esp_lcd_panel_dev_config_t *panel_dev_config,
                                   esp_lcd_panel_handle_t *ret_panel)
{
    esp_err_t ret = ESP_OK;
    st7306_panel_t *st7306 = NULL;

    ESP_GOTO_ON_FALSE(io && panel_dev_config && ret_panel, ESP_ERR_INVALID_ARG, err, TAG, "invalid argument");

    st7306 = (st7306_panel_t *)calloc(1, sizeof(st7306_panel_t));
    ESP_GOTO_ON_FALSE(st7306, ESP_ERR_NO_MEM, err, TAG, "no mem for st7306 panel");

    if (panel_dev_config->reset_gpio_num >= 0) {
        gpio_config_t io_conf = {
            .pin_bit_mask = 1ULL << panel_dev_config->reset_gpio_num,
            .mode = GPIO_MODE_OUTPUT,
        };
        ESP_GOTO_ON_ERROR(gpio_config(&io_conf), err, TAG, "configure GPIO for RST line failed");
    }

    st7306->io = io;
    st7306->reset_gpio_num = panel_dev_config->reset_gpio_num;
    st7306->reset_level = panel_dev_config->flags.reset_active_high;
    st7306->hor_res = 400;
    st7306->ver_res = 300;
    st7306->col_start = 0x05;
    st7306->col_end = 0x36;
    st7306->row_start = 0x00;
    st7306->row_end = 0xC7;
    st7306->madctl_val = 0x00;
    st7306->colmod_val = 0x11;

    if (panel_dev_config->vendor_config) {
        const st7306_vendor_config_t *vendor_config = (const st7306_vendor_config_t *)panel_dev_config->vendor_config;
        st7306->init_cmds = vendor_config->init_cmds;
        st7306->init_cmds_size = vendor_config->init_cmds_size;
        if (vendor_config->hor_res) {
            st7306->hor_res = vendor_config->hor_res;
        }
        if (vendor_config->ver_res) {
            st7306->ver_res = vendor_config->ver_res;
        }
        st7306->col_start = vendor_config->col_start ? vendor_config->col_start : st7306->col_start;
        st7306->col_end = vendor_config->col_end ? vendor_config->col_end : st7306->col_end;
        st7306->row_start = vendor_config->row_start;
        st7306->row_end = vendor_config->row_end ? vendor_config->row_end : st7306->row_end;
    }

    st7306->base.del = panel_st7306_del;
    st7306->base.reset = panel_st7306_reset;
    st7306->base.init = panel_st7306_init;
    st7306->base.draw_bitmap = panel_st7306_draw_bitmap;
    st7306->base.invert_color = panel_st7306_invert_color;
    st7306->base.set_gap = panel_st7306_set_gap;
    st7306->base.mirror = panel_st7306_mirror;
    st7306->base.swap_xy = panel_st7306_swap_xy;
    st7306->base.disp_on_off = panel_st7306_disp_on_off;

    *ret_panel = &st7306->base;
    ESP_LOGI(TAG, "LCD panel create success");
    return ESP_OK;

err:
    if (st7306) {
        if (panel_dev_config->reset_gpio_num >= 0) {
            gpio_reset_pin(panel_dev_config->reset_gpio_num);
        }
        free(st7306);
    }
    return ret;
}

static esp_err_t panel_st7306_del(esp_lcd_panel_t *panel)
{
    st7306_panel_t *st7306 = __containerof(panel, st7306_panel_t, base);

    if (st7306->reset_gpio_num >= 0) {
        gpio_reset_pin(st7306->reset_gpio_num);
    }
    free(st7306);
    return ESP_OK;
}

static esp_err_t panel_st7306_reset(esp_lcd_panel_t *panel)
{
    st7306_panel_t *st7306 = __containerof(panel, st7306_panel_t, base);

    if (st7306->reset_gpio_num >= 0) {
        gpio_set_level(st7306->reset_gpio_num, !st7306->reset_level);
        vTaskDelay(pdMS_TO_TICKS(10));
        gpio_set_level(st7306->reset_gpio_num, st7306->reset_level);
        vTaskDelay(pdMS_TO_TICKS(10));
        gpio_set_level(st7306->reset_gpio_num, !st7306->reset_level);
        vTaskDelay(pdMS_TO_TICKS(10));
    } else {
        ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(st7306->io, ST7306_SWRESET, NULL, 0), TAG, "send command failed");
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    return ESP_OK;
}

static esp_err_t panel_st7306_init(esp_lcd_panel_t *panel)
{
    st7306_panel_t *st7306 = __containerof(panel, st7306_panel_t, base);
    ESP_LOGI(TAG, "OSPTEK GitHub init: HPM, B2=12, MADCTL=48, C0=12/0A, inversion OFF");
    const st7306_lcd_init_cmd_t *init_cmds = st7306->init_cmds;
    uint16_t init_cmds_size = st7306->init_cmds_size;

    if (!init_cmds) {
        init_cmds = st7306_default_init_cmds;
        init_cmds_size = sizeof(st7306_default_init_cmds) / sizeof(st7306_default_init_cmds[0]);
    }

    for (int i = 0; i < init_cmds_size; i++) {
        switch (init_cmds[i].cmd) {
        case ST7306_MADCTL:
            if (init_cmds[i].data && init_cmds[i].data_bytes) {
                st7306->madctl_val = ((const uint8_t *)init_cmds[i].data)[0];
            }
            break;
        case ST7306_COLMOD:
            if (init_cmds[i].data && init_cmds[i].data_bytes) {
                st7306->colmod_val = ((const uint8_t *)init_cmds[i].data)[0];
            }
            break;
        default:
            break;
        }

        ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(st7306->io, init_cmds[i].cmd,
                                                      init_cmds[i].data, init_cmds[i].data_bytes),
                            TAG, "send command failed");
        if (init_cmds[i].delay_ms != 0) {
            vTaskDelay(pdMS_TO_TICKS(init_cmds[i].delay_ms));
        }
    }

    return ESP_OK;
}

static esp_err_t panel_st7306_draw_bitmap(esp_lcd_panel_t *panel, int x_start, int y_start, int x_end, int y_end, const void *color_data)
{
    st7306_panel_t *st7306 = __containerof(panel, st7306_panel_t, base);
    ESP_RETURN_ON_FALSE(color_data, ESP_ERR_INVALID_ARG, TAG, "invalid color data");
    ESP_RETURN_ON_FALSE((x_start < x_end) && (y_start < y_end), ESP_ERR_INVALID_ARG, TAG, "invalid area");
    ESP_RETURN_ON_FALSE(st7306->x_gap == 0 && st7306->y_gap == 0, ESP_ERR_NOT_SUPPORTED, TAG, "gap is not supported");
    ESP_RETURN_ON_FALSE(x_start == 0 && y_start == 0 && x_end == st7306->hor_res && y_end == st7306->ver_res,
                        ESP_ERR_NOT_SUPPORTED, TAG, "only full-frame draw is supported");

    uint8_t caset[] = {st7306->col_start, st7306->col_end};
    uint8_t raset[] = {st7306->row_start, st7306->row_end};

    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(st7306->io, ST7306_CASET, caset, sizeof(caset)), TAG, "send command failed");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(st7306->io, ST7306_RASET, raset, sizeof(raset)), TAG, "send command failed");

    size_t len = ((size_t)st7306->hor_res * st7306->ver_res) / 4;
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_color(st7306->io, ST7306_RAMWR, color_data, len), TAG, "send color failed");
    return ESP_OK;
}

static esp_err_t panel_st7306_invert_color(esp_lcd_panel_t *panel, bool invert_color_data)
{
    st7306_panel_t *st7306 = __containerof(panel, st7306_panel_t, base);
    return esp_lcd_panel_io_tx_param(st7306->io, invert_color_data ? ST7306_INVON : ST7306_INVOFF, NULL, 0);
}

static esp_err_t panel_st7306_mirror(esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y)
{
    st7306_panel_t *st7306 = __containerof(panel, st7306_panel_t, base);

    if (mirror_x) {
        st7306->madctl_val |= LCD_CMD_MX_BIT;
    } else {
        st7306->madctl_val &= ~LCD_CMD_MX_BIT;
    }
    if (mirror_y) {
        st7306->madctl_val |= LCD_CMD_MY_BIT;
    } else {
        st7306->madctl_val &= ~LCD_CMD_MY_BIT;
    }
    return esp_lcd_panel_io_tx_param(st7306->io, ST7306_MADCTL, &st7306->madctl_val, 1);
}

static esp_err_t panel_st7306_swap_xy(esp_lcd_panel_t *panel, bool swap_axes)
{
    st7306_panel_t *st7306 = __containerof(panel, st7306_panel_t, base);

    if (swap_axes) {
        st7306->madctl_val |= LCD_CMD_MV_BIT;
    } else {
        st7306->madctl_val &= ~LCD_CMD_MV_BIT;
    }
    return esp_lcd_panel_io_tx_param(st7306->io, ST7306_MADCTL, &st7306->madctl_val, 1);
}

static esp_err_t panel_st7306_set_gap(esp_lcd_panel_t *panel, int x_gap, int y_gap)
{
    st7306_panel_t *st7306 = __containerof(panel, st7306_panel_t, base);
    ESP_RETURN_ON_FALSE(x_gap == 0 && y_gap == 0, ESP_ERR_NOT_SUPPORTED, TAG, "gap is not supported");
    st7306->x_gap = x_gap;
    st7306->y_gap = y_gap;
    return ESP_OK;
}

static esp_err_t panel_st7306_disp_on_off(esp_lcd_panel_t *panel, bool on_off)
{
    st7306_panel_t *st7306 = __containerof(panel, st7306_panel_t, base);

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
    on_off = !on_off;
#endif

    return esp_lcd_panel_io_tx_param(st7306->io, on_off ? ST7306_DISPON : ST7306_DISPOFF, NULL, 0);
}

esp_err_t esp_rlcd_st7306_frame_buffer_init(st7306_frame_buffer_t *fb,
                                            uint16_t width,
                                            uint16_t height,
                                            st7306_pixel_mapping_t mapping,
                                            uint32_t caps)
{
    ESP_RETURN_ON_FALSE(fb && width && height, ESP_ERR_INVALID_ARG, TAG, "invalid frame buffer argument");
    ESP_RETURN_ON_FALSE((mapping == ST7306_PIXEL_MAPPING_LANDSCAPE && width == 400 && height == 300) ||
                        (mapping == ST7306_PIXEL_MAPPING_PORTRAIT && width == 300 && height == 400),
                        ESP_ERR_INVALID_ARG, TAG, "HSD BWR requires 400x300 landscape or 300x400 portrait");
    memset(fb, 0, sizeof(*fb));

    size_t pixel_count = (size_t)width * height;
    fb->buffer_size = pixel_count / 4;
    fb->width = width;
    fb->height = height;
    fb->mapping = mapping;

    fb->buffer = (uint8_t *)heap_caps_malloc(fb->buffer_size, caps);
    if (!fb->buffer && caps != MALLOC_CAP_8BIT) {
        ESP_LOGW(TAG, "frame buffer alloc with caps 0x%" PRIx32 " failed, fallback to MALLOC_CAP_8BIT", caps);
        fb->buffer = (uint8_t *)heap_caps_malloc(fb->buffer_size, MALLOC_CAP_8BIT);
    }
    if (!fb->buffer) {
        esp_rlcd_st7306_frame_buffer_deinit(fb);
        return ESP_ERR_NO_MEM;
    }

    fb->index_lut = (uint16_t *)heap_caps_malloc(pixel_count * sizeof(uint16_t), caps);
    fb->bit_lut = (uint8_t *)heap_caps_malloc(pixel_count * sizeof(uint8_t), caps);
    if ((!fb->index_lut || !fb->bit_lut) && caps != MALLOC_CAP_8BIT) {
        free(fb->index_lut);
        free(fb->bit_lut);
        fb->index_lut = (uint16_t *)heap_caps_malloc(pixel_count * sizeof(uint16_t), MALLOC_CAP_8BIT);
        fb->bit_lut = (uint8_t *)heap_caps_malloc(pixel_count * sizeof(uint8_t), MALLOC_CAP_8BIT);
    }
    if (!fb->index_lut || !fb->bit_lut) {
        free(fb->index_lut);
        free(fb->bit_lut);
        fb->index_lut = NULL;
        fb->bit_lut = NULL;
        ESP_LOGW(TAG, "lookup table alloc failed, fallback to realtime pixel mapping");
        esp_rlcd_st7306_frame_buffer_clear(fb, ST7306_COLOR_WHITE);
        return ESP_OK;
    }

    for (uint16_t y = 0; y < height; y++) {
        for (uint16_t x = 0; x < width; x++) {
            uint16_t index;
            uint8_t mask;
            st7306_get_pixel_location(fb, x, y, &index, &mask);
            size_t lut_pos = (size_t)y * width + x;
            fb->index_lut[lut_pos] = index;
            fb->bit_lut[lut_pos] = mask;
        }
    }

    esp_rlcd_st7306_frame_buffer_clear(fb, ST7306_COLOR_WHITE);
    return ESP_OK;
}

void esp_rlcd_st7306_frame_buffer_deinit(st7306_frame_buffer_t *fb)
{
    if (!fb) {
        return;
    }
    free(fb->buffer);
    free(fb->index_lut);
    free(fb->bit_lut);
    memset(fb, 0, sizeof(*fb));
}

void esp_rlcd_st7306_frame_buffer_clear(st7306_frame_buffer_t *fb, uint8_t color)
{
    if (!fb || !fb->buffer) {
        return;
    }
    /* Verified uniform native bytes: WHITE=33, BLACK=FF, RED=CC. */
    uint8_t packed = color == ST7306_COLOR_BLACK ? 0xFF :
                     color == ST7306_COLOR_RED ? 0xCC : 0x33;
    memset(fb->buffer, packed, fb->buffer_size);
}

esp_err_t esp_rlcd_st7306_frame_buffer_set_pixel(st7306_frame_buffer_t *fb, uint16_t x, uint16_t y, uint8_t color)
{
    ESP_RETURN_ON_FALSE(fb && fb->buffer, ESP_ERR_INVALID_ARG, TAG, "invalid frame buffer");
    ESP_RETURN_ON_FALSE(x < fb->width && y < fb->height, ESP_ERR_INVALID_ARG, TAG, "pixel out of range");
    ESP_RETURN_ON_FALSE(color == ST7306_COLOR_WHITE || color == ST7306_COLOR_BLACK || color == ST7306_COLOR_RED,
                        ESP_ERR_INVALID_ARG, TAG, "invalid BWR color");

    uint16_t index;
    uint8_t mask;
    if (fb->index_lut && fb->bit_lut) {
        size_t lut_pos = (size_t)y * fb->width + x;
        index = fb->index_lut[lut_pos];
        mask = fb->bit_lut[lut_pos];
    } else {
        st7306_get_pixel_location(fb, x, y, &index, &mask);
    }

    uint8_t value = color == ST7306_COLOR_BLACK ? mask :
                    color == ST7306_COLOR_RED ? (mask & 0xCC) : (mask & 0x33);
    fb->buffer[index] = (fb->buffer[index] & (uint8_t)~mask) | value;

    return ESP_OK;
}

esp_err_t esp_rlcd_st7306_frame_buffer_draw_line(st7306_frame_buffer_t *fb,
                                                 int x0,
                                                 int y0,
                                                 int x1,
                                                 int y1,
                                                 uint8_t color)
{
    ESP_RETURN_ON_FALSE(fb && fb->buffer, ESP_ERR_INVALID_ARG, TAG, "invalid frame buffer");

    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    while (true) {
        if (st7306_clip_point(fb, x0, y0)) {
            ESP_RETURN_ON_ERROR(esp_rlcd_st7306_frame_buffer_set_pixel(fb, x0, y0, color), TAG, "set pixel failed");
        }

        if (x0 == x1 && y0 == y1) {
            break;
        }

        int e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }

    return ESP_OK;
}

esp_err_t esp_rlcd_st7306_frame_buffer_draw_rect(st7306_frame_buffer_t *fb,
                                                 int x,
                                                 int y,
                                                 int width,
                                                 int height,
                                                 uint8_t color)
{
    ESP_RETURN_ON_FALSE(fb && fb->buffer, ESP_ERR_INVALID_ARG, TAG, "invalid frame buffer");
    ESP_RETURN_ON_FALSE(width > 0 && height > 0, ESP_ERR_INVALID_ARG, TAG, "invalid rectangle size");

    int x2 = x + width - 1;
    int y2 = y + height - 1;

    ESP_RETURN_ON_ERROR(esp_rlcd_st7306_frame_buffer_draw_line(fb, x, y, x2, y, color), TAG, "draw top line failed");
    if (height > 1) {
        ESP_RETURN_ON_ERROR(esp_rlcd_st7306_frame_buffer_draw_line(fb, x, y2, x2, y2, color), TAG, "draw bottom line failed");
    }
    if (height > 2) {
        ESP_RETURN_ON_ERROR(esp_rlcd_st7306_frame_buffer_draw_line(fb, x, y + 1, x, y2 - 1, color), TAG, "draw left line failed");
        if (width > 1) {
            ESP_RETURN_ON_ERROR(esp_rlcd_st7306_frame_buffer_draw_line(fb, x2, y + 1, x2, y2 - 1, color), TAG, "draw right line failed");
        }
    }

    return ESP_OK;
}

esp_err_t esp_rlcd_st7306_frame_buffer_fill_rect(st7306_frame_buffer_t *fb,
                                                 int x,
                                                 int y,
                                                 int width,
                                                 int height,
                                                 uint8_t color)
{
    ESP_RETURN_ON_FALSE(fb && fb->buffer, ESP_ERR_INVALID_ARG, TAG, "invalid frame buffer");
    ESP_RETURN_ON_FALSE(width > 0 && height > 0, ESP_ERR_INVALID_ARG, TAG, "invalid rectangle size");

    int x_start = x < 0 ? 0 : x;
    int y_start = y < 0 ? 0 : y;
    int x_end = x + width;
    int y_end = y + height;

    if (x_end > fb->width) {
        x_end = fb->width;
    }
    if (y_end > fb->height) {
        y_end = fb->height;
    }

    if (x_start >= x_end || y_start >= y_end) {
        return ESP_OK;
    }

    for (int yy = y_start; yy < y_end; yy++) {
        for (int xx = x_start; xx < x_end; xx++) {
            ESP_RETURN_ON_ERROR(esp_rlcd_st7306_frame_buffer_set_pixel(fb, xx, yy, color), TAG, "set pixel failed");
        }
    }

    return ESP_OK;
}

esp_err_t esp_rlcd_st7306_frame_buffer_flush(esp_lcd_panel_handle_t panel, const st7306_frame_buffer_t *fb)
{
    ESP_RETURN_ON_FALSE(panel && fb && fb->buffer, ESP_ERR_INVALID_ARG, TAG, "invalid flush argument");
    return esp_lcd_panel_draw_bitmap(panel, 0, 0, fb->width, fb->height, fb->buffer);
}

static bool st7306_clip_point(const st7306_frame_buffer_t *fb, int x, int y)
{
    return x >= 0 && y >= 0 && x < fb->width && y < fb->height;
}

static void st7306_get_pixel_location(const st7306_frame_buffer_t *fb, uint16_t x, uint16_t y, uint16_t *index, uint8_t *mask)
{
    uint8_t red_bit;
    /* Match fill_cell_r270(): each byte contains four display pixels,
     * with BLACK masks A0/50/0A/05 and RED masks 80/40/08/04.
     */
    if (fb->mapping == ST7306_PIXEL_MAPPING_LANDSCAPE) {
        *index = (x / 2) * (fb->height / 2) + y / 2;
        red_bit = 7 - ((y & 1) * 4 + (x & 1));
    } else {
        *index = (y / 2) * (fb->width / 2) + x / 2;
        red_bit = 7 - ((x & 1) * 4 + (y & 1));
    }
    *mask = (1U << red_bit) | (1U << (red_bit - 2));
}

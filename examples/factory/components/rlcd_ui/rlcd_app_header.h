#pragma once
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RLCD_APP_HEADER_HEIGHT 48

/* Use solid panel colors so emphasis remains legible without grayscale. */
static inline lv_color_t rlcd_ui_accent(void) { return lv_color_hex(0xFF0000); }

static inline void rlcd_ui_primary_button(lv_obj_t *button)
{
    lv_obj_set_style_bg_color(button, rlcd_ui_accent(), 0);
    lv_obj_set_style_bg_opa(button, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(button, rlcd_ui_accent(), 0);
    lv_obj_set_style_text_color(button, lv_color_white(), 0);
    lv_obj_set_style_bg_color(button, lv_color_black(), LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(button, lv_color_white(), LV_STATE_DISABLED);
    lv_obj_set_style_border_color(button, lv_color_black(), LV_STATE_DISABLED);
    lv_obj_set_style_text_color(button, lv_color_black(), LV_STATE_DISABLED);
    lv_obj_add_flag(button, LV_OBJ_FLAG_STATE_TRICKLE);
    for (uint32_t i = 0; i < lv_obj_get_child_count(button); ++i) {
        lv_obj_t *child = lv_obj_get_child(button, i);
        lv_obj_add_flag(child, LV_OBJ_FLAG_STATE_TRICKLE);
        lv_obj_set_style_text_color(child, lv_color_white(), 0);
        lv_obj_set_style_text_color(child, lv_color_black(), LV_STATE_DISABLED);
    }
}

/* Neutral secondary actions leave red emphasis to the primary control. */
static inline void rlcd_ui_secondary_button(lv_obj_t *button)
{
    lv_obj_set_style_bg_color(button, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(button, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(button, 1, 0);
    lv_obj_set_style_border_color(button, lv_color_black(), 0);
    lv_obj_set_style_text_color(button, lv_color_black(), 0);
    lv_obj_set_style_bg_color(button, lv_color_black(), LV_STATE_PRESSED);
    lv_obj_set_style_border_color(button, lv_color_black(), LV_STATE_PRESSED);
    lv_obj_set_style_text_color(button, lv_color_white(), LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(button, lv_color_white(), LV_STATE_DISABLED);
    lv_obj_set_style_border_color(button, lv_color_black(), LV_STATE_DISABLED);
    lv_obj_set_style_text_color(button, lv_color_black(), LV_STATE_DISABLED);
    lv_obj_add_flag(button, LV_OBJ_FLAG_STATE_TRICKLE);
    for (uint32_t i = 0; i < lv_obj_get_child_count(button); ++i) {
        lv_obj_t *child = lv_obj_get_child(button, i);
        lv_obj_add_flag(child, LV_OBJ_FLAG_STATE_TRICKLE);
        lv_obj_set_style_text_color(child, lv_color_black(), 0);
        lv_obj_set_style_text_color(child, lv_color_white(), LV_STATE_PRESSED);
        lv_obj_set_style_text_color(child, lv_color_black(), LV_STATE_DISABLED);
    }
}

typedef struct {
    lv_obj_t *root;
    lv_obj_t *title;
    lv_obj_t *back;
} rlcd_app_header_t;

LV_IMAGE_DECLARE(rlcd_header_weather);
LV_IMAGE_DECLARE(rlcd_header_music);
LV_IMAGE_DECLARE(rlcd_header_2048);
LV_IMAGE_DECLARE(rlcd_header_photos);
LV_IMAGE_DECLARE(rlcd_header_sensor);
LV_IMAGE_DECLARE(rlcd_header_settings);
LV_IMAGE_DECLARE(rlcd_header_button_test);
LV_IMAGE_DECLARE(rlcd_header_mic_test);

rlcd_app_header_t rlcd_app_header_create(lv_obj_t *parent, const char *title,
                                       const lv_image_dsc_t *icon,
                                       lv_event_cb_t on_back, void *context);

#ifdef __cplusplus
}
#endif

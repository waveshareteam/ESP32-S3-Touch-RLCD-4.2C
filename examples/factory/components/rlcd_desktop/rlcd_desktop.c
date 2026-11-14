#include "rlcd_desktop.h"
#include "desktop_faces.h"
#include "bsp/esp-bsp.h"
#include "wifi_manager.h"
#include <stdio.h>
#include <string.h>

LV_IMAGE_DECLARE(esp_brookesia_image_small_status_bar_wifi_close_20_20);
LV_IMAGE_DECLARE(esp_brookesia_image_small_status_bar_wifi_level3_20_20);
LV_FONT_DECLARE(rlcd_font_status_16);

static struct {
    lv_obj_t *root, *clock, *face, *wifi, *battery_value, *battery_icon, *battery_fill;
    lv_timer_t *timer;
    bool wifi_initialized, connected, clock_initialized;
    uint32_t clock_tick;
} ui;

static void text_if_changed(lv_obj_t *label, const char *value)
{
    if (label && strcmp(lv_label_get_text(label), value)) lv_label_set_text(label, value);
}

static lv_obj_t *label(int x, int y, int width, const char *text, const lv_font_t *font)
{
    lv_obj_t *obj = lv_label_create(ui.root);
    lv_obj_set_pos(obj, x, y);
    lv_obj_set_width(obj, width);
    lv_obj_set_style_text_font(obj, font, 0);
    lv_obj_set_style_text_color(obj, lv_color_black(), 0);
    lv_obj_set_style_text_opa(obj, LV_OPA_COVER, 0);
    lv_label_set_text(obj, text);
    return obj;
}

static void poll(lv_timer_t *timer)
{
    (void)timer;
    if (!ui.root || lv_obj_get_screen(ui.root) != lv_screen_active()) return;
    if (!ui.clock_initialized || lv_tick_elaps(ui.clock_tick)>=1000) {
        struct tm local;
        if (wifi_manager_get_local_time(&local))
            rlcd_desktop_set_time(local.tm_hour,local.tm_min);
        else
            rlcd_desktop_set_time(-1,-1);
        bsp_battery_info_t battery;
        const bool battery_valid = bsp_battery_get_info(&battery) == ESP_OK;
        rlcd_desktop_set_battery(battery_valid ? battery.percent : -1, false);
        ui.clock_tick=lv_tick_get();
        ui.clock_initialized=true;
    }
    const lv_image_dsc_t *face = (bsp_user_button_get_press_count() & 1) ?
        &desktop_face_neutral : &desktop_face_smile;
    if (lv_image_get_src(ui.face) != face) lv_image_set_src(ui.face, face);
    const bool connected = wifi_manager_is_connected();
    if (!ui.wifi_initialized || connected != ui.connected) {
        lv_image_set_src(ui.wifi, connected ?
            &esp_brookesia_image_small_status_bar_wifi_level3_20_20 :
            &esp_brookesia_image_small_status_bar_wifi_close_20_20);
        ui.connected = connected;
        ui.wifi_initialized = true;
    }
}

static void deleted(lv_event_t *event)
{
    if (lv_event_get_target(event) != ui.root) return;
    if (ui.timer) lv_timer_delete(ui.timer);
    memset(&ui, 0, sizeof(ui));
}

void rlcd_desktop_destroy(void)
{
    if (ui.root) lv_obj_delete(ui.root);
}

bool rlcd_desktop_create(lv_obj_t *desktop)
{
    if (!desktop) return false;
    rlcd_desktop_destroy();
    ui.root = lv_obj_create(desktop);
    lv_obj_remove_style_all(ui.root);
    lv_obj_set_pos(ui.root, 0, 0);
    lv_obj_set_size(ui.root, 400, 40);
    lv_obj_set_style_bg_color(ui.root, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(ui.root, LV_OPA_COVER, 0);
    lv_obj_remove_flag(ui.root, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(ui.root, deleted, LV_EVENT_DELETE, NULL);
    lv_obj_t *rule = lv_obj_create(ui.root);
    lv_obj_remove_style_all(rule);
    lv_obj_set_pos(rule, 0, 39);
    lv_obj_set_size(rule, 400, 1);
    lv_obj_set_style_bg_color(rule, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(rule, LV_OPA_COVER, 0);
    lv_obj_remove_flag(rule, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t *accent = lv_obj_create(ui.root);
    lv_obj_remove_style_all(accent);
    lv_obj_set_pos(accent, 12, 36);
    lv_obj_set_size(accent, 64, 3);
    lv_obj_set_style_bg_color(accent, lv_color_hex(0xFF0000), 0);
    lv_obj_set_style_bg_opa(accent, LV_OPA_COVER, 0);
    lv_obj_remove_flag(accent, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    ui.clock = label(16, 11, 74, "--:--", &rlcd_font_status_16);

    ui.face = lv_image_create(ui.root);
    lv_image_set_src(ui.face, &desktop_face_smile);
    lv_obj_set_pos(ui.face, 184, 3);
    lv_obj_remove_flag(ui.face, LV_OBJ_FLAG_CLICKABLE);
    ui.wifi = lv_image_create(ui.root);
    lv_obj_set_pos(ui.wifi, 286, 10);
    lv_obj_set_style_image_recolor(ui.wifi, lv_color_black(), 0);
    lv_obj_set_style_image_recolor_opa(ui.wifi, LV_OPA_COVER, 0);
    ui.battery_value = label(311, 11, 46, "--%", &rlcd_font_status_16);
    lv_obj_set_style_text_align(ui.battery_value, LV_TEXT_ALIGN_RIGHT, 0);
    ui.battery_icon = lv_obj_create(ui.root);
    lv_obj_remove_style_all(ui.battery_icon);
    lv_obj_set_pos(ui.battery_icon, 362, 13);
    lv_obj_set_size(ui.battery_icon, 23, 13);
    lv_obj_set_style_border_color(ui.battery_icon, lv_color_black(), 0);
    lv_obj_set_style_border_width(ui.battery_icon, 1, 0);
    lv_obj_set_style_radius(ui.battery_icon, 2, 0);
    lv_obj_remove_flag(ui.battery_icon, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    ui.battery_fill = lv_obj_create(ui.battery_icon);
    lv_obj_remove_style_all(ui.battery_fill);
    lv_obj_set_pos(ui.battery_fill, 1, 1);
    lv_obj_set_size(ui.battery_fill, 1, 9);
    lv_obj_set_style_bg_color(ui.battery_fill, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(ui.battery_fill, LV_OPA_COVER, 0);
    lv_obj_remove_flag(ui.battery_fill, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t *terminal = lv_obj_create(ui.root);
    lv_obj_remove_style_all(terminal);
    lv_obj_set_pos(terminal, 385, 17);
    lv_obj_set_size(terminal, 2, 5);
    lv_obj_set_style_bg_color(terminal, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(terminal, LV_OPA_COVER, 0);
    lv_obj_remove_flag(terminal, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    ui.timer = lv_timer_create(poll, 50, NULL);
    if (!ui.timer) { rlcd_desktop_destroy(); return false; }
    poll(NULL);
    return true;
}

void rlcd_desktop_set_time(int hour, int minute)
{
    char text[8] = "--:--";
    if (hour >= 0 && hour < 24 && minute >= 0 && minute < 60)
        snprintf(text, sizeof(text), "%02d:%02d", hour, minute);
    text_if_changed(ui.clock, text);
}

void rlcd_desktop_set_battery(int percent, bool charging)
{
    (void)charging;
    char text[8] = "--%";
    const bool valid = percent >= 0 && percent <= 100;
    if (valid) {
        snprintf(text, sizeof(text), "%d%%", percent);
    }
    text_if_changed(ui.battery_value, text);
    const lv_color_t color = valid && percent <= 20 ? lv_color_hex(0xFF0000) : lv_color_black();
    if (ui.battery_value && !lv_color_eq(lv_obj_get_style_text_color(ui.battery_value, 0), color))
        lv_obj_set_style_text_color(ui.battery_value, color, 0);
    if (ui.battery_fill) {
        if (!lv_color_eq(lv_obj_get_style_bg_color(ui.battery_fill, 0), color))
            lv_obj_set_style_bg_color(ui.battery_fill, color, 0);
        lv_obj_set_flag(ui.battery_fill, LV_OBJ_FLAG_HIDDEN, !valid || percent == 0);
        if (valid && percent > 0) lv_obj_set_width(ui.battery_fill, (19 * percent + 99) / 100);
    }
}


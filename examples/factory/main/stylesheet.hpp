#pragma once

#include "esp_brookesia.hpp"
#include "bsp/display.h"

// Reuse the base Brookesia layout, overriding only the settings needed for the 400x300 three-color display.
inline esp_brookesia::systems::phone::Stylesheet make_rlcd_stylesheet()
{
    using namespace esp_brookesia;
    using namespace esp_brookesia::gui;
    using namespace esp_brookesia::systems::phone;

    Stylesheet sheet = STYLESHEET_320_240_DARK;
    sheet.core.name = "400x300 Black White Red";
    sheet.core.screen_size = StyleSize::RECT(BSP_LCD_H_RES, BSP_LCD_V_RES);
    sheet.core.display.background.color = StyleColor::COLOR(0xFFFFFF);
    sheet.core.display.background.wallpaper_image_resource = StyleImage::IMAGE(nullptr);
    sheet.core.manager.app.max_running_num = 4;
    sheet.core.manager.flags.enable_app_save_snapshot = 0;

    // Keep desktop and app management in the touch example; the drawing board uses the full 400x300 visual area.
    sheet.display.flags.enable_status_bar = 0;
    // The desktop owns its header; APP visual areas remain the full 400x300.
    sheet.display.flags.enable_app_launcher_flex_size = 0;
    sheet.display.flags.enable_navigation_bar = 0;
    sheet.display.flags.enable_recents_screen = 0;
    // Keep Brookesia navigation gestures, as in the C5-2.8 reference project.
    sheet.manager.flags.enable_gesture = 1;
    // Hide the bottom gesture indicator while keeping navigation gestures such as swiping up from the bottom.
    sheet.manager.gesture.flags.enable_indicator_bars[static_cast<int>(Gesture::IndicatorBarType::BOTTOM)] = 0;
    sheet.manager.flags.enable_recents_screen_snapshot_drag = 0;

    auto &launcher = sheet.display.app_launcher.data;
    launcher.table.default_num = 2;
    launcher.main.y_start = 40;
    launcher.main.size = StyleSize::RECT(400, 260);
    launcher.table.size = StyleSize::RECT(400, 260);
    launcher.icon.main.size = StyleSize::RECT(100, 118);
    // Display three-color icons at their native size, including when pressed, to keep their outlines sharp.
    launcher.icon.image.default_size = StyleSize::SQUARE(64);
    launcher.icon.image.press_size = StyleSize::SQUARE(64);
    launcher.icon.label.text_font = StyleFont::SIZE(14);
    launcher.icon.label.text_color = StyleColor::COLOR(0x000000);
    // A compact red marker shows the active app page without animation.
    launcher.indicator.spot_active_background_color = StyleColor::COLOR(0xFF0000);
    launcher.indicator.spot_inactive_background_color = StyleColor::COLOR(0x000000);
    launcher.indicator.main_size = StyleSize::RECT(400, 6);
    launcher.indicator.spot_inactive_size = StyleSize::SQUARE(5);
    launcher.indicator.spot_active_size = StyleSize::RECT(18, 5);
    launcher.indicator.main_layout_column_pad = 7;
    launcher.indicator.main_layout_bottom_offset = 3;
    return sheet;
}

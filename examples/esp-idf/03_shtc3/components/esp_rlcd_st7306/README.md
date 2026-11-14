# ST7306 Reflective LCD Driver

[English](README.md) | [简体中文](README_ZH.md)

## Overview

ESP-IDF panel driver and RGB565 packer for the 400×300 landscape black/white/red ST7306 panel. The default initialization uses HPM with inversion disabled.

## Hardware

SPI mode 0 at 40 MHz, with CS, DC and reset signals. The Factory BSP owns the product pin assignments and touch configuration.

## Usage

Create the panel with `esp_lcd_new_panel_st7306()` and `bits_per_pixel=2`, then reset, initialize, enable the display and disable inversion. Convert a full little-endian RGB565 frame with `st7306_rgb565_pack()` and `swap_red_white=false`; send the 30,000-byte result using `esp_lcd_panel_draw_bitmap(panel, 0, 0, 400, 300, frame)`.

## Expected Result

The packer selects the nearest black, white or red for each pixel. Logical white/red/black codes are 1/2/3; uniform native bytes are respectively `0x33`/`0xCC`/`0xFF`. Each byte packs a 2×2 pixel group in the panel's subpixel order.

## Notes

Only full-frame draws are supported. Keep the native buffer valid until the panel IO color-transfer callback completes. The RGB565 source and packed destination must not overlap.

Initialization reference: [OSPTEK ST7306 demo](https://github.com/osptek/display-rlcd-4.2inch-300x400/tree/main/examples/esp32s3-st7306-spi-demo), `Initial_ST7306()`, source blob `fee3aaa5ddf063f58dcd3f5ffe8ce42d6bded6e5`.

The Factory BSP mirrors the packed output vertically while retaining the native horizontal direction and the existing board touch coordinates.

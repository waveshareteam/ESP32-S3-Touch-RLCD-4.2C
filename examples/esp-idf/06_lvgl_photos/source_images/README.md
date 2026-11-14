# Gallery previews

[English](README.md) | [简体中文](README_ZH.md)

The two 400 x 300 PNGs preview the embedded images in `../main/assets/photos.c`. They are not read at runtime. Each LVGL I2 array contains a 16-byte BGRA palette and 300 rows of 100 bytes. Indices 0, 1 and 2 are black, white and red; index 3 is unused white.

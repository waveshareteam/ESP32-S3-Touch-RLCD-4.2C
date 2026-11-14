# 相册预览

[English](README.md) | [简体中文](README_ZH.md)

两张 400×300 PNG 用于预览 `../main/assets/photos.c` 中内置的图片，运行时不读取 PNG。每个 LVGL I2 数组包含 16 字节 BGRA 调色板，以及 300 行、每行 100 字节的像素数据。索引 0、1、2 分别为黑、白、红，索引 3 为保留白色。

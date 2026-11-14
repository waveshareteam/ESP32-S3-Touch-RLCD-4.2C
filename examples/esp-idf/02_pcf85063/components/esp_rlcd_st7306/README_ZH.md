# ST7306 反射式液晶驱动

[English](README.md) | [简体中文](README_ZH.md)

## 功能

为横屏 400×300 黑白红三色 ST7306 面板提供 ESP-IDF 面板驱动与 RGB565 转换，默认初始化使用 HPM 模式，关闭反色。

## 硬件

使用 SPI 模式 0、40 MHz，连接 CS、DC 和复位信号。产品引脚和触摸配置由 Factory BSP 管理。

## 使用

调用 `esp_lcd_new_panel_st7306()`，设置 `bits_per_pixel=2`，依次复位、初始化、开屏并关闭反色。使用 `st7306_rgb565_pack()` 将一整帧小端 RGB565 转为 30,000 字节原生数据，参数 `swap_red_white=false`，再调用 `esp_lcd_panel_draw_bitmap(panel, 0, 0, 400, 300, frame)` 发送。

## 预期效果

转换器为每个像素选择最接近的黑、白、红颜色。白、红、黑逻辑编码分别为 1、2、3，对应纯色原生字节为 `0x33`、`0xCC`、`0xFF`，每个字节按面板子像素顺序保存 2×2 个像素。

## 说明

仅支持整帧刷新，原生缓冲必须保持有效直到面板 IO 传输完成回调触发，RGB565 输入与压缩输出缓冲不可重叠。

初始化参考 [OSPTEK ST7306 示例](https://github.com/osptek/display-rlcd-4.2inch-300x400/tree/main/examples/esp32s3-st7306-spi-demo)的 `Initial_ST7306()`，源文件 blob 为 `fee3aaa5ddf063f58dcd3f5ffe8ce42d6bded6e5`。

Factory BSP 在发送前将原生画面上下翻转，保留原生水平方向和原有主板触摸坐标。

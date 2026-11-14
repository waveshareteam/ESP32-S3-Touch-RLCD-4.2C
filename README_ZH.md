<div align="center">
  <h1>ESP32-S3-Touch-RLCD-4.2C</h1>
  <p><strong>400×300 ST7306 黑白红三色反射式触摸屏开发板</strong></p>
  <p><a href="README.md">English</a> | <strong>简体中文</strong></p>
  <p>
    <a href="https://github.com/waveshareteam/ESP32-S3-Touch-RLCD-4.2C/actions/workflows/build.yml"><img alt="Build" src="https://github.com/waveshareteam/ESP32-S3-Touch-RLCD-4.2C/actions/workflows/build.yml/badge.svg"></a>
    <a href="https://github.com/waveshareteam/ESP32-S3-Touch-RLCD-4.2C/releases/latest"><img alt="Latest Release" src="https://img.shields.io/github/v/release/waveshareteam/ESP32-S3-Touch-RLCD-4.2C"></a>
    <a href="LICENSE"><img alt="License" src="https://img.shields.io/github/license/waveshareteam/ESP32-S3-Touch-RLCD-4.2C"></a>
  </p>
  <p>
    <a href="https://www.waveshare.com/">🌐 产品</a> &middot;
    <a href="firmware/">📦 固件</a> &middot;
    <a href="examples/esp-idf/">🧩 ESP-IDF</a> &middot;
    <a href="examples/arduino/examples/">🔧 Arduino</a> &middot;
    <a href="examples/factory/README_ZH.md">📚 资料</a>
  </p>
</div>

---

## ✨ 概述

本仓库提供功能和界面对应的 ESP-IDF、Arduino 01–06 示例，以及出厂应用源码。反射式屏幕采用 **ST7306** 驱动，显示**黑、白、红三色**，使用 **2 bit/像素编码**，没有背光。

| 项目 | 器件 / 接口 |
| --- | --- |
| MCU | ESP32-S3 |
| 存储 | 16 MB Flash、8 MB OPI PSRAM |
| 显示 | 4.2 英寸，400×300，ST7306，SPI |
| 显示颜色 / 编码 | 黑、白、红三色；每像素 2 bit |
| 触摸 | CST816，I2C |
| RTC / 温湿度 | PCF85063A / SHTC3 |
| 音频 | ES8311、ES7210、双麦克风录放 |
| 存储卡 | microSD，SDMMC 1-bit |

屏幕实际显示三种颜色，2 bit 表示像素编码位数，并非四级灰度。LVGL 内部使用 RGB565 渲染，由 BSP 转换为 ST7306 原生格式后发送到屏幕；像素映射见[显示驱动说明](examples/factory/components/esp_rlcd_st7306/README_ZH.md)。

## 📦 快速开始

1. 从 [firmware](firmware/) 下载最新的出厂固件 BIN。
2. 使用 USB 数据线连接开发板，按照[烧录说明](firmware/README_ZH.md)操作。完整 BIN 烧录地址为 **0x0**，选择 ESP32-S3、16 MB Flash、DIO 和 80 MHz。
3. 重新上电，在环境光下查看桌面，轻点图标打开应用。
4. 按照[出厂应用使用说明](examples/factory/README_ZH.md)完成配网、准备 SD 卡并体验各应用。

出厂应用源码位于 [examples/factory](examples/factory/)。自行修改时，选择下方示例并按对应 README 编译烧录。

## 🧪 示例

### ESP-IDF

| 示例 | 功能 |
| --- | --- |
| [01_lvgl](examples/esp-idf/01_lvgl/README_ZH.md) | LVGL 触摸示例 |
| [02_pcf85063](examples/esp-idf/02_pcf85063/README_ZH.md) | RTC 时钟 |
| [03_shtc3](examples/esp-idf/03_shtc3/README_ZH.md) | 温湿度显示 |
| [04_sd_card_test](examples/esp-idf/04_sd_card_test/README_ZH.md) | microSD 读写测试 |
| [05_mic](examples/esp-idf/05_mic/README_ZH.md) | 双麦克风录放测试 |
| [06_lvgl_photos](examples/esp-idf/06_lvgl_photos/README_ZH.md) | 左右滑动相册 |

### Arduino

| 示例 | 功能 |
| --- | --- |
| [01_lvgl](examples/arduino/examples/01_lvgl/README_ZH.md) | LVGL 触摸示例 |
| [02_pcf85063](examples/arduino/examples/02_pcf85063/README_ZH.md) | RTC 时钟 |
| [03_shtc3](examples/arduino/examples/03_shtc3/README_ZH.md) | 温湿度显示 |
| [04_sd_card_test](examples/arduino/examples/04_sd_card_test/README_ZH.md) | microSD 读写测试 |
| [05_mic](examples/arduino/examples/05_mic/README_ZH.md) | 双麦克风录放测试 |
| [06_lvgl_photos](examples/arduino/examples/06_lvgl_photos/README_ZH.md) | 左右滑动相册 |

Arduino 公共库位于 [examples/arduino/libraries](examples/arduino/libraries/)，所有草图共用 `board_config` 和 LVGL 9.5.0。

## 🛠️ 开发环境

| 框架 | 版本 | 示例数 |
| --- | --- | ---: |
| ESP-IDF | v6.0.2 | 6 |
| Arduino-ESP32 | 3.3.12 | 6 |
| LVGL | 9.5.0 | 两个框架共用版本 |

版本与开发板参数见 [.github/product.json](.github/product.json)。

## 📚 资料

- [原理图](hardware/schematic/)
- [尺寸图与 3D 模型](hardware/mechanical/)：与 4.2B 共用，保留原始图纸文件名。
- [官方产品网站](https://www.waveshare.com/)
- [ESP-IDF 示例](examples/esp-idf/)
- [Arduino 示例](examples/arduino/examples/)

## 🤝 支持

- 产品使用及硬件问题：请联系[微雪技术支持](https://www.waveshare.net/wiki/技术支持)。

## 📄 许可证

第一方示例采用 Apache License 2.0，详见 [LICENSE](LICENSE)；第三方库与字体保留各自许可证。

# 固件

[English](README.md) | [简体中文](README_ZH.md)

适用于 ESP32-S3-Touch-RLCD-4.2C 的出厂固件，可直接下载烧录。每个 BIN 都是从 `0x0` 烧录的完整合并镜像，文件名中的日期用于区分版本；后续更新时保留旧版本。

| 文件 | 用途 |
| --- | --- |
| [ESP32-S3-Touch-RLCD-4.2C_FactoryOnly_20261114.bin](ESP32-S3-Touch-RLCD-4.2C_FactoryOnly_20261114.bin) | 黑白红三色出厂固件，包含画板、语音聊天、天气、音乐、2048、相册、传感器、设置、按键测试、屏幕测试和麦克风测试 11 个应用 |

芯片为 **ESP32-S3**，Flash 为 **16 MB**，烧录模式 **DIO**，Flash 频率 **80 MHz**。本固件使用 **ESP-IDF v6.0.2**。

下载 BIN 后，在文件所在目录打开终端，使用支持数据传输的 USB 线连接开发板，执行：

```sh
python -m pip install esptool
python -m esptool --chip esp32s3 erase-flash
python -m esptool --chip esp32s3 write-flash --flash-mode dio --flash-freq 80m --flash-size 16MB 0x0 ESP32-S3-Touch-RLCD-4.2C_FactoryOnly_20261114.bin
```

烧录完成后重新上电，在环境光下查看屏幕；这块屏幕没有背光。按照[出厂固件使用说明](../examples/factory/README_ZH.md)完成配网并体验各应用。

擦除或烧录这份完整镜像会清除保存的 Wi-Fi、音量和激活信息，之后需要重新设置；SD 卡里的文件不受影响。镜像包含所需的语音模型，NVS 与用户存储区域为空白，不带入设备上保存的配置或个人图片。

出厂固件源码位于 [examples/factory](../examples/factory/)。

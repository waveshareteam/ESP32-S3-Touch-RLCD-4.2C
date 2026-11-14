# Firmware

[English](README.md) | [简体中文](README_ZH.md)

Ready-to-flash factory firmware for the ESP32-S3-Touch-RLCD-4.2C. Each BIN is a complete merged image flashed at `0x0`. The date in its filename identifies the version; older versions are retained when updates are added.

| File | Purpose |
| --- | --- |
| [ESP32-S3-Touch-RLCD-4.2C_FactoryOnly_20261114.bin](ESP32-S3-Touch-RLCD-4.2C_FactoryOnly_20261114.bin) | Black/white/red factory firmware with Drawing, AIChats, Weather, Music, 2048, Photos, Sensor, Settings, Button Test, Display Test, and Mic Test |

The target is **ESP32-S3**, with **16 MB Flash**, **DIO**, and **80 MHz**. This firmware uses **ESP-IDF v6.0.2**.

Download the BIN and open a terminal in its directory. Connect the board using a USB data cable, then run:

```sh
python -m pip install esptool
python -m esptool --chip esp32s3 erase-flash
python -m esptool --chip esp32s3 write-flash --flash-mode dio --flash-freq 80m --flash-size 16MB 0x0 ESP32-S3-Touch-RLCD-4.2C_FactoryOnly_20261114.bin
```

After flashing, restart the board and view the screen under ambient light; it has no backlight. Follow the [factory firmware user guide](../examples/factory/README.md) to set up Wi-Fi and try the apps.

Erasing or flashing this complete image clears saved Wi-Fi, volume, and activation information. Set these up again afterward. Files on the SD card are unaffected. The image includes the required speech model; NVS and user storage are blank, with no saved device configuration or personal images.

The factory firmware source is in [examples/factory](../examples/factory/).

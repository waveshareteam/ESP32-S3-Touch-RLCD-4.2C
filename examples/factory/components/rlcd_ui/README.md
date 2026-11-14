# RLCD App Header

[English](README.md) | [简体中文](README_ZH.md)

## Overview

Shared 400×48 white header with black titles and icons, an outlined black back button, and a short red accent along the bottom rule. Weather, Music, 2048, Photos, Sensor, Settings, Button Test and Mic Test use this component.

## Hardware

ESP32-S3-Touch-RLCD-4.2C with the 400×300 black/white/red ST7306 screen.

## Usage

Call `rlcd_app_header_create()` under the LVGL lock and supply the existing back callback. Content starts at y=48; the full-screen photo viewer and Display Test keep their own layouts.

## Expected Result

Native 32×32 I1 icons use black glyphs on white. `rlcd_ui_primary_button()` styles primary actions red with white captions, black when pressed, and white with black captions when disabled. `rlcd_ui_secondary_button()` uses white backgrounds with black outlines and captions, and switches to black with white captions when pressed.

## Notes

Children belong to the supplied parent and are released with it. This component has no worker, hardware access or navigation state; the application owns its callbacks.

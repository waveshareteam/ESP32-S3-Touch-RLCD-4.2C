# Photo album

[English](README.md) | [简体中文](README_ZH.md)

## What It Does

Decodes JPEG files from the SD card and displays them as black, white and red images in a 400x300 full-screen viewer. Red areas remain red; other colors use grayscale dithering. Other aspect ratios are fitted with white margins.

## Hardware Required

ESP32-S3-Touch-RLCD-4.2C with the peripherals used by this application.

## How to Run

Build and flash the [factory application](../../README.md). Copy the `photo` folder from `sdcard_example` to the root of a FAT32 SD card, insert the card, then open Photos. The two 400x300 baseline JPG samples show a dog with a red scarf and a red plum landscape. Older firmware converts photos to black and white; flash the updated app before testing red artwork.

## Expected Result

Open a photo from the list, swipe left or right to change photos, and swipe up to return to the list. The viewer has no header; the list's back arrow closes the app.

## Troubleshooting

See the [Chinese usage guide](README_ZH.md) for the existing detailed component notes.

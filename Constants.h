#ifndef Constants_h
#define Constants_h

#include <Windows.h>
#define CLASS_NAME "ScreenshotCapture"
#define HOTKEY_MOD MOD_CONTROL | MOD_SHIFT
#define HOTKEY_VK 0x5A
#define SELECTION_COLOR pixel(0xFF8000)
#define LINE_WIDTH 2
#define LINE_DISPLAY_WIDTH (LINE_WIDTH << 1) + 1
#define SELECTION_ALPHA 191

#define NOTHING 0x00
#define SELECTION 0x01

#endif
#pragma once
#include <Arduino.h>

// ---------------- PCF8574 on CN1 connector ----------------
// CN1 pinout (JST 1.25mm 4-pin): GND, GPIO22, GPIO27, 3V3
#define I2C_SDA 27
#define I2C_SCL 22
#define I2C_FREQ 100000      // PCF8574 is rated for 100 kHz (a full 8-pin scan ~4 ms).
                             // Most parts run fine at 400000 for a faster scan.

// ---------------- Touch (XPT2046, its own SPI bus) ----------------
#define XPT_IRQ  36
#define XPT_MOSI 32
#define XPT_MISO 39
#define XPT_CLK  25
#define XPT_CS   33

// Raw touch range -> screen mapping (landscape, rotation 1).
// Watch the serial monitor: every touch prints its raw x/y. Touch the four
// corners and adjust these if taps land in the wrong spot.
#define TOUCH_X_MIN 200
#define TOUCH_X_MAX 3700
#define TOUCH_Y_MIN 240
#define TOUCH_Y_MAX 3800
#define TOUCH_SWAP_XY 0
#define TOUCH_INVERT_X 0
#define TOUCH_INVERT_Y 0

// ---------------- Display ----------------
#define SCREEN_W 320
#define SCREEN_H 240
#define TFT_ROTATION 1       // 1 = landscape, USB on the right. Use 3 to flip.

// ---------------- On-board extras ----------------
#define LED_R 4              // RGB LED, active LOW
#define LED_G 16
#define LED_B 17
#define SPEAKER_PIN 26       // "SPEAK" JST connector (on-board amp)
#define ENABLE_BEEP 1

// ---------------- Scanning ----------------
#define MAX_PINS 8
#define MAX_KEYS 32
#define SCAN_INTERVAL_MS 4
#define SETTLE_US 30         // wait after driving a line low before reading back
#define DEBOUNCE_SCANS 4     // identical scans needed before a change is accepted (~16 ms)

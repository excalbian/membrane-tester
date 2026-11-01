// Membrane Switch Tester
// ESP32-2432S028R (CYD) + PCF8574 + LVGL 9
//
// Display/touch/LVGL plumbing lives here; the tester logic is in model.cpp
// and the screens are in ui.cpp.

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <lvgl.h>

#include "config.h"
#include "model.h"
#include "ui.h"

static TFT_eSPI tft;
static SPIClass touchSpi(VSPI);
static XPT2046_Touchscreen ts(XPT_CS, XPT_IRQ);

#define BUF_LINES 24
static uint16_t buf1[SCREEN_W * BUF_LINES];
static uint16_t buf2[SCREEN_W * BUF_LINES];

// ---------------------------------------------------------------- feedback
#define BEEP_CH 4
static uint32_t beepUntil = 0;

void fb_beep(uint16_t freq, uint16_t ms) {
#if ENABLE_BEEP
  ledcWriteTone(BEEP_CH, freq);
  beepUntil = millis() + ms;
  if (!beepUntil) beepUntil = 1;
#endif
}

void fb_led(bool r, bool g, bool b) {   // on-board RGB LED is active LOW
  digitalWrite(LED_R, r ? LOW : HIGH);
  digitalWrite(LED_G, g ? LOW : HIGH);
  digitalWrite(LED_B, b ? LOW : HIGH);
}

// ---------------------------------------------------------------- LVGL glue
static uint32_t tick_cb() { return millis(); }

static void flush_cb(lv_display_t* disp, const lv_area_t* area, uint8_t* px) {
  uint32_t w = lv_area_get_width(area);
  uint32_t h = lv_area_get_height(area);
  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors((uint16_t*)px, w * h, true);   // true = swap bytes for the panel
  tft.endWrite();
  lv_display_flush_ready(disp);
}

static void touch_cb(lv_indev_t* indev, lv_indev_data_t* data) {
  if (!(ts.tirqTouched() && ts.touched())) {
    data->state = LV_INDEV_STATE_RELEASED;
    return;
  }
  TS_Point p = ts.getPoint();
  int rx = p.x, ry = p.y;
#if TOUCH_SWAP_XY
  int t = rx; rx = ry; ry = t;
#endif
  int x = map(rx, TOUCH_X_MIN, TOUCH_X_MAX, 0, SCREEN_W - 1);
  int y = map(ry, TOUCH_Y_MIN, TOUCH_Y_MAX, 0, SCREEN_H - 1);
#if TOUCH_INVERT_X
  x = SCREEN_W - 1 - x;
#endif
#if TOUCH_INVERT_Y
  y = SCREEN_H - 1 - y;
#endif
  data->point.x = constrain(x, 0, SCREEN_W - 1);
  data->point.y = constrain(y, 0, SCREEN_H - 1);
  data->state = LV_INDEV_STATE_PRESSED;

  // Raw values for calibration (see config.h), throttled
  static uint32_t lastPrint = 0;
  if (millis() - lastPrint > 250) {
    lastPrint = millis();
    Serial.printf("touch raw x=%d y=%d -> screen %d,%d\n", p.x, p.y,
                  (int)data->point.x, (int)data->point.y);
  }
}

// ---------------------------------------------------------------- setup/loop
void setup() {
  Serial.begin(115200);
  Serial.println("\nMembrane switch tester");

  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  fb_led(false, false, false);

#if ENABLE_BEEP
  ledcSetup(BEEP_CH, 2000, 8);
  ledcAttachPin(SPEAKER_PIN, BEEP_CH);
  ledcWriteTone(BEEP_CH, 0);
#endif

  tft.init();
  tft.setRotation(TFT_ROTATION);
  tft.fillScreen(TFT_BLACK);
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, TFT_BACKLIGHT_ON);

  touchSpi.begin(XPT_CLK, XPT_MISO, XPT_MOSI, XPT_CS);
  ts.begin(touchSpi);
  ts.setRotation(TFT_ROTATION);

  lv_init();
  lv_tick_set_cb(tick_cb);

  lv_display_t* disp = lv_display_create(SCREEN_W, SCREEN_H);
  lv_display_set_flush_cb(disp, flush_cb);
  lv_display_set_buffers(disp, buf1, buf2, sizeof(buf1), LV_DISPLAY_RENDER_MODE_PARTIAL);

  lv_indev_t* indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, touch_cb);

  if (scanner_begin())
    Serial.printf("PCF8574 found at 0x%02X\n", scanner_address());
  else
    Serial.println("PCF8574 not found - check wiring on CN1 (SDA=27, SCL=22)");

  profiles_seed_defaults();
  ui_init();
}

void loop() {
  static uint32_t lastScan = 0;
  uint32_t now = millis();

  if (now - lastScan >= SCAN_INTERVAL_MS) {
    lastScan = now;
    if (scanner_poll()) ui_on_scan(scanner_state());
  }

#if ENABLE_BEEP
  if (beepUntil && (int32_t)(now - beepUntil) >= 0) {
    ledcWriteTone(BEEP_CH, 0);
    beepUntil = 0;
  }
#endif

  lv_timer_handler();
  delay(1);
}

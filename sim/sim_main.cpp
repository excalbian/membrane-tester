// Desktop simulator for the membrane switch tester.
//
// Runs the real ui.cpp + model.cpp (scanner, profiles, test and learn logic)
// against a simulated PCF8574. The mouse is the touchscreen; the keyboard is
// the membrane switch under test. Saved profiles persist in sim_nvs.bin.
//
//   4x4 matrix mode (default)        common-contact mode (Tab)
//   rows = pins 1-4, cols = 5-8      pin 1 common, keys = pins 2-8
//     1 2 3 4                          1 2 3 4 5 6 7
//     Q W E R
//     A S D F
//     Z X C V
//
//   Hold keys to press switch keys (several at once is fine - ghosting is real)
//   K  toggle a short between pins 2 and 3
//   G  toggle pin 8 shorted to ground
//   P  unplug / re-plug the PCF8574
//   H  print this help

#include <SDL2/SDL.h>
#include <lvgl.h>
#include <Wire.h>
#include <Preferences.h>
#include <stdio.h>
#include "ui.h"

// ---- state the stubs expect
uint8_t sim_pairs[8];
uint8_t sim_gnd = 0;
bool sim_present = true;
TwoWire Wire;
std::map<std::string, std::vector<uint8_t>> sim_nvs;

uint32_t millis() { return SDL_GetTicks(); }

static lv_display_t* g_disp;
static bool g_common = false, g_short = false;
static const char* NVS_FILE = "sim_nvs.bin";

// ---- feedback hooks from ui.cpp
void fb_beep(uint16_t freq, uint16_t ms) { printf("  beep %u Hz, %u ms\n", freq, ms); }
static const char* g_led = "off";
void fb_led(bool r, bool g, bool b) { g_led = r ? "RED" : g ? "GREEN" : b ? "BLUE" : "off"; }

// ---- NVS persistence
static void nvs_load() {
  FILE* f = fopen(NVS_FILE, "rb");
  if (!f) return;
  uint32_t n, kl, vl;
  if (fread(&n, 4, 1, f) == 1)
    for (uint32_t i = 0; i < n; i++) {
      if (fread(&kl, 4, 1, f) != 1) break;
      std::string k(kl, '\0');
      if (fread(&k[0], 1, kl, f) != kl || fread(&vl, 4, 1, f) != 1) break;
      std::vector<uint8_t> v(vl);
      if (vl && fread(v.data(), 1, vl, f) != vl) break;
      sim_nvs[k] = v;
    }
  fclose(f);
}
static void nvs_save() {
  FILE* f = fopen(NVS_FILE, "wb");
  if (!f) return;
  uint32_t n = sim_nvs.size();
  fwrite(&n, 4, 1, f);
  for (auto& kv : sim_nvs) {
    uint32_t kl = kv.first.size(), vl = kv.second.size();
    fwrite(&kl, 4, 1, f); fwrite(kv.first.data(), 1, kl, f);
    fwrite(&vl, 4, 1, f); fwrite(kv.second.data(), 1, vl, f);
  }
  fclose(f);
}

// ---- virtual membrane switch
static void close_pair(int a, int b) { sim_pairs[a] |= 1 << b; sim_pairs[b] |= 1 << a; }

static void read_keyboard() {
  static const SDL_Scancode matrix[4][4] = {
    {SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3, SDL_SCANCODE_4},
    {SDL_SCANCODE_Q, SDL_SCANCODE_W, SDL_SCANCODE_E, SDL_SCANCODE_R},
    {SDL_SCANCODE_A, SDL_SCANCODE_S, SDL_SCANCODE_D, SDL_SCANCODE_F},
    {SDL_SCANCODE_Z, SDL_SCANCODE_X, SDL_SCANCODE_C, SDL_SCANCODE_V}};
  static const SDL_Scancode common[7] = {SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3, SDL_SCANCODE_4,
                                         SDL_SCANCODE_5, SDL_SCANCODE_6, SDL_SCANCODE_7};
  const Uint8* k = SDL_GetKeyboardState(nullptr);
  memset(sim_pairs, 0, sizeof(sim_pairs));
  if (g_common) {
    for (int i = 0; i < 7; i++) if (k[common[i]]) close_pair(0, i + 1);
  } else {
    for (int r = 0; r < 4; r++)
      for (int c = 0; c < 4; c++) if (k[matrix[r][c]]) close_pair(r, 4 + c);
  }
  if (g_short) close_pair(1, 2);
}

static void update_title() {
  char t[160];
  snprintf(t, sizeof(t), "Membrane tester sim | switch: %s%s%s%s | LED %s | H = help",
           g_common ? "common (1-7)" : "4x4 matrix", g_short ? " | SHORT 2-3" : "",
           sim_gnd ? " | pin 8 GND" : "", sim_present ? "" : " | PCF UNPLUGGED", g_led);
  lv_sdl_window_set_title(g_disp, t);
}

static void help() {
  printf("\nKeyboard = the switch under test (hold keys to press):\n"
         "  4x4 matrix (rows pins 1-4, cols 5-8):  1 2 3 4 / Q W E R / A S D F / Z X C V\n"
         "  Tab  switch to common-contact mode (pin 1 common, keys 1-7 = pins 2-8)\n"
         "  K    toggle short between pins 2 and 3\n"
         "  G    toggle pin 8 shorted to ground\n"
         "  P    unplug / replug the PCF8574\n"
         "Mouse = touchscreen. Profiles are saved to %s.\n\n", NVS_FILE);
}

static void toggles() {
  static bool prev[4];
  const Uint8* k = SDL_GetKeyboardState(nullptr);
  const SDL_Scancode keys[4] = {SDL_SCANCODE_TAB, SDL_SCANCODE_K, SDL_SCANCODE_G, SDL_SCANCODE_P};
  for (int i = 0; i < 4; i++) {
    bool now = k[keys[i]];
    if (now && !prev[i]) {
      switch (i) {
        case 0: g_common = !g_common; printf("switch mode: %s\n", g_common ? "common" : "4x4 matrix"); break;
        case 1: g_short = !g_short; printf("short 2-3: %s\n", g_short ? "on" : "off"); break;
        case 2: sim_gnd ^= 1 << 7; printf("pin 8 to GND: %s\n", sim_gnd ? "on" : "off"); break;
        case 3: sim_present = !sim_present; printf("PCF8574 %s\n", sim_present ? "plugged in" : "unplugged"); break;
      }
    }
    prev[i] = now;
  }
  if (k[SDL_SCANCODE_H]) { static uint32_t last; if (millis() - last > 500) { help(); last = millis(); } }
}

int main(int, char**) {
  nvs_load();
  atexit(nvs_save);

  lv_init();
  lv_tick_set_cb([]() -> uint32_t { return SDL_GetTicks(); });
  g_disp = lv_sdl_window_create(SCREEN_W, SCREEN_H);
  lv_sdl_window_set_zoom(g_disp, 2);
  lv_sdl_mouse_create();

  scanner_begin();
  profiles_seed_defaults();
  ui_init();
  help();

  uint32_t lastScan = 0, lastTitle = 0;
  while (true) {
    lv_timer_handler();           // also pumps SDL events (window close exits)
    toggles();
    read_keyboard();
    uint32_t now = millis();
    if (now - lastScan >= SCAN_INTERVAL_MS) {
      lastScan = now;
      if (scanner_poll()) ui_on_scan(scanner_state());
    }
    if (now - lastTitle > 200) { lastTitle = now; update_title(); }
    SDL_Delay(2);
  }
}

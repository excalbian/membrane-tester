// Simulator LVGL config: the firmware's settings plus the SDL display driver.
#ifndef SIM_LV_CONF_H
#define SIM_LV_CONF_H
#include "../include/lv_conf.h"
#define LV_USE_SDL 1
#define LV_SDL_INCLUDE_PATH <SDL2/SDL.h>
#define LV_SDL_RENDER_MODE LV_DISPLAY_RENDER_MODE_DIRECT
#define LV_SDL_BUF_COUNT 1
#define LV_SDL_FULLSCREEN 0
#define LV_SDL_DIRECT_EXIT 1
#endif

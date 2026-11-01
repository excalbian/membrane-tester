#pragma once
#include "model.h"

void ui_init();
void ui_on_scan(const ConnState& s);   // call when the debounced pin state changes

// Feedback hooks, implemented in main.cpp
void fb_beep(uint16_t freq, uint16_t ms);
void fb_led(bool r, bool g, bool b);

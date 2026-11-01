// Simulated PCF8574 for the desktop simulator: answers at 0x20 and reports
// pin levels from the "virtual membrane switch" driven by the keyboard.
#pragma once
#include <stdint.h>
// Simulated PCF8574 at 0x20 with a set of closed pin pairs (the "DUT").
extern uint8_t sim_pairs[8];     // sim_pairs[i] bit j => pins i,j closed
extern uint8_t sim_gnd;          // pins shorted to ground
extern bool sim_present;
class TwoWire {
  uint8_t addr = 0, out = 0xFF;
 public:
  void begin(int, int, uint32_t) {}
  void setTimeOut(int) {}
  void beginTransmission(uint8_t a) { addr = a; }
  void write(uint8_t v) { out = v; }
  uint8_t endTransmission() { return (sim_present && addr == 0x20) ? 0 : 2; }
  int requestFrom(int a, int) { return (sim_present && a == 0x20) ? 1 : 0; }
  int read() {
    // net = transitive closure of closed pairs; a net is low if any member driven low or GND-shorted
    uint8_t low = (uint8_t)~out | sim_gnd;
    bool changed = true;
    while (changed) { changed = false;
      for (int i = 0; i < 8; i++) if (low & (1 << i)) for (int j = 0; j < 8; j++)
        if ((sim_pairs[i] & (1 << j)) && !(low & (1 << j))) { low |= 1 << j; changed = true; } }
    return (uint8_t)~low;
  }
};
extern TwoWire Wire;

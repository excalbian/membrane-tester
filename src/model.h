#pragma once
#include <Arduino.h>
#include "config.h"

// ============================================================================
//  Connectivity scanner (PCF8574)
// ============================================================================
// Every scan drives one expander line low at a time (the rest sit at the
// PCF8574's weak ~100 uA pull-up) and reads which other lines follow it low.
// The result is a full pin-to-pin connectivity map, which works the same for
// common-contact layouts, matrices, or anything else.

struct ConnState {
  uint8_t adj[MAX_PINS];  // adj[i] bit j set => pin i is connected to pin j
  uint8_t leak;           // pins that read LOW with nothing driven (short to GND)

  bool connected(uint8_t a, uint8_t b) const { return adj[a] & (1u << b); }
  bool any() const;
  bool equals(const ConnState& o) const;
  void clear();
};

bool scanner_begin();             // finds the PCF8574 (0x20-0x27, 0x38-0x3F)
bool scanner_ok();
uint8_t scanner_address();
// Call often. Returns true when the debounced state changes.
bool scanner_poll();
const ConnState& scanner_state();

// Groups of mutually connected pins. Returns group count; group[i] = id or -1.
int conn_groups(const ConnState& s, int8_t group[MAX_PINS]);

// ============================================================================
//  Profiles: an expected set of keys, each key = a pair of pins
// ============================================================================
enum Topology : uint8_t { TOPO_COMMON = 0, TOPO_MATRIX = 1, TOPO_CUSTOM = 2 };

struct Profile {
  char name[24];
  uint8_t nPins;
  uint8_t nKeys;
  uint8_t gridCols;
  uint8_t topo;
  uint8_t keyA[MAX_KEYS];
  uint8_t keyB[MAX_KEYS];
};

void profile_make_common(Profile& p, uint8_t nPins, uint8_t commonPin);
void profile_make_matrix(Profile& p, uint8_t nPins, uint8_t nRows);
void profile_finalize_learned(Profile& p);   // detect topology + grid shape
void profile_describe(const Profile& p, char* buf, size_t len);
int profile_find_key(const Profile& p, uint8_t a, uint8_t b);
uint8_t auto_grid_cols(uint8_t nKeys);

// NVS storage
#define MAX_PROFILES 20
int profiles_count();
bool profiles_load(int idx, Profile& p);
bool profiles_save(const Profile& p);        // overwrites a profile with the same name
bool profiles_delete(int idx);

// ============================================================================
//  Test session
// ============================================================================
#define MAX_FAULTS 12

struct TestSession {
  Profile prof;
  bool active[MAX_KEYS];      // currently closed
  bool tested[MAX_KEYS];      // seen closed at least once
  bool armed[MAX_KEYS];       // false for keys already closed when test started
  bool stuck[MAX_KEYS];       // closed at start (stuck / shorted)
  uint16_t presses[MAX_KEYS];
  uint8_t testedCount;
  uint8_t faultA[MAX_FAULTS], faultB[MAX_FAULTS];
  uint8_t nFaults;            // unique unexpected pin pairs seen
  uint8_t leak;               // accumulated pins shorted to GND
  bool idleDirty;             // something was connected when the test started
};

enum : uint32_t {
  EV_PRESS    = 1u << 0,
  EV_RELEASE  = 1u << 1,
  EV_FAULT    = 1u << 2,
  EV_COMPLETE = 1u << 3,
};

void session_start(TestSession& t, const Profile& p, const ConnState& now);
uint32_t session_update(TestSession& t, const ConnState& s);
bool session_has_faults(const TestSession& t);
bool session_pass(const TestSession& t);
void session_fault_text(const TestSession& t, char* buf, size_t len);

// ============================================================================
//  Learn session: press keys one at a time to build a profile
// ============================================================================
enum LearnEvent { LE_NONE, LE_ADDED, LE_DUPLICATE, LE_MULTI, LE_FULL };

struct LearnSession {
  Profile prof;
  bool waitRelease;
  int lastDup;
};

void learn_start(LearnSession& l);
LearnEvent learn_update(LearnSession& l, const ConnState& s);
void learn_undo(LearnSession& l);

// ============================================================================
//  Profile helpers used by the UI
// ============================================================================
void profiles_seed_defaults();                 // first boot: add common templates
bool profiles_find(const char* name, Profile& p);
void profiles_set_last(const char* name);
bool profiles_get_last(Profile& p);
// "1-3, 2-5-7" style description of what's connected right now
void conn_text(const ConnState& s, char* buf, size_t len, const char* sep);

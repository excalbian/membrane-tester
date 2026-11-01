#include "model.h"
#include <Wire.h>
#include <Preferences.h>
#include <string.h>

// ============================================================================
//  ConnState
// ============================================================================
bool ConnState::any() const {
  for (int i = 0; i < MAX_PINS; i++) if (adj[i]) return true;
  return leak != 0;
}
bool ConnState::equals(const ConnState& o) const {
  return memcmp(adj, o.adj, sizeof(adj)) == 0 && leak == o.leak;
}
void ConnState::clear() { memset(adj, 0, sizeof(adj)); leak = 0; }

int conn_groups(const ConnState& s, int8_t group[MAX_PINS]) {
  for (int i = 0; i < MAX_PINS; i++) group[i] = -1;
  int n = 0;
  for (int i = 0; i < MAX_PINS; i++) {
    if (group[i] >= 0 || !s.adj[i]) continue;
    // adj is already a transitive closure (driving one pin low pulls the
    // whole connected net low), so one pass is enough.
    group[i] = n;
    for (int j = 0; j < MAX_PINS; j++)
      if (s.adj[i] & (1u << j)) group[j] = n;
    n++;
  }
  return n;
}

// ============================================================================
//  Scanner
// ============================================================================
static uint8_t s_addr = 0;
static bool s_ok = false;
static ConnState s_stable, s_candidate;
static uint8_t s_candCount = 0;
static uint32_t s_lastProbe = 0;

static bool pcf_write(uint8_t v) {
  Wire.beginTransmission(s_addr);
  Wire.write(v);
  return Wire.endTransmission() == 0;
}
static int pcf_read() {
  if (Wire.requestFrom((int)s_addr, 1) != 1) return -1;
  return Wire.read();
}

static bool probe() {
  static const uint8_t bases[] = {0x20, 0x38};  // PCF8574, PCF8574A
  for (uint8_t base : bases) {
    for (uint8_t a = base; a < base + 8; a++) {
      Wire.beginTransmission(a);
      if (Wire.endTransmission() == 0) {
        s_addr = a;
        s_ok = pcf_write(0xFF);
        if (s_ok) return true;
      }
    }
  }
  s_ok = false;
  return false;
}

bool scanner_begin() {
  Wire.begin(I2C_SDA, I2C_SCL, I2C_FREQ);
  Wire.setTimeOut(20);
  s_stable.clear();
  s_candidate.clear();
  return probe();
}

bool scanner_ok() { return s_ok; }
uint8_t scanner_address() { return s_ok ? s_addr : 0; }
const ConnState& scanner_state() { return s_stable; }

static bool raw_scan(ConnState& s) {
  s.clear();
  if (!pcf_write(0xFF)) return false;
  delayMicroseconds(SETTLE_US);
  int v = pcf_read();
  if (v < 0) return false;
  s.leak = (~v) & 0xFF;

  for (uint8_t i = 0; i < MAX_PINS; i++) {
    if (s.leak & (1u << i)) continue;          // already low, can't test it
    if (!pcf_write((uint8_t)~(1u << i))) return false;
    delayMicroseconds(SETTLE_US);
    v = pcf_read();
    if (v < 0) return false;
    uint8_t c = (uint8_t)(~v) & ~(1u << i) & ~s.leak;
    s.adj[i] |= c;
    for (uint8_t j = 0; j < MAX_PINS; j++)
      if (c & (1u << j)) s.adj[j] |= (1u << i);
  }
  pcf_write(0xFF);
  return true;
}

bool scanner_poll() {
  if (!s_ok) {
    if (millis() - s_lastProbe > 1000) {
      s_lastProbe = millis();
      probe();
    }
    if (s_stable.any()) { s_stable.clear(); return true; }
    return false;
  }

  ConnState raw;
  if (!raw_scan(raw)) {
    s_ok = false;               // expander vanished; retry probing
    s_lastProbe = millis();
    s_stable.clear();
    return true;
  }

  if (raw.equals(s_candidate)) {
    if (s_candCount < 255) s_candCount++;
  } else {
    s_candidate = raw;
    s_candCount = 1;
  }
  if (s_candCount >= DEBOUNCE_SCANS && !s_candidate.equals(s_stable)) {
    s_stable = s_candidate;
    return true;
  }
  return false;
}

// ============================================================================
//  Profiles
// ============================================================================
uint8_t auto_grid_cols(uint8_t n) {
  if (n <= 4) return n ? n : 1;
  if (n <= 8) return (n + 1) / 2;
  if (n <= 12) return 4;
  if (n <= 20) return 5;
  return 6;
}

void profile_make_common(Profile& p, uint8_t nPins, uint8_t commonPin) {
  memset(&p, 0, sizeof(p));
  snprintf(p.name, sizeof(p.name), "%u-pin common", nPins);
  p.nPins = nPins;
  p.topo = TOPO_COMMON;
  for (uint8_t i = 0; i < nPins; i++) {
    if (i == commonPin) continue;
    p.keyA[p.nKeys] = commonPin;
    p.keyB[p.nKeys] = i;
    p.nKeys++;
  }
  p.gridCols = p.nKeys <= 7 ? p.nKeys : auto_grid_cols(p.nKeys);
}

void profile_make_matrix(Profile& p, uint8_t nPins, uint8_t nRows) {
  memset(&p, 0, sizeof(p));
  uint8_t nCols = nPins - nRows;
  snprintf(p.name, sizeof(p.name), "%ux%u matrix", nRows, nCols);
  p.nPins = nPins;
  p.topo = TOPO_MATRIX;
  for (uint8_t r = 0; r < nRows; r++)
    for (uint8_t c = 0; c < nCols && p.nKeys < MAX_KEYS; c++) {
      p.keyA[p.nKeys] = r;
      p.keyB[p.nKeys] = nRows + c;
      p.nKeys++;
    }
  p.gridCols = nCols;
}

int profile_find_key(const Profile& p, uint8_t a, uint8_t b) {
  for (int k = 0; k < p.nKeys; k++)
    if ((p.keyA[k] == a && p.keyB[k] == b) || (p.keyA[k] == b && p.keyB[k] == a))
      return k;
  return -1;
}

// Figure out whether a learned key set is a common-contact or matrix layout.
static bool detect_common(const Profile& p, uint8_t& common) {
  if (p.nKeys < 2) return false;
  for (uint8_t cand : {p.keyA[0], p.keyB[0]}) {
    bool all = true;
    for (int k = 0; k < p.nKeys && all; k++)
      if (p.keyA[k] != cand && p.keyB[k] != cand) all = false;
    if (all) { common = cand; return true; }
  }
  return false;
}

// Two-colour the pin graph. Returns true if bipartite (i.e. it's a matrix).
static bool detect_matrix(const Profile& p, uint8_t& rowMask, uint8_t& colMask) {
  int8_t side[MAX_PINS];
  for (int i = 0; i < MAX_PINS; i++) side[i] = -1;
  while (true) {
    bool progress;
    do {
      progress = false;
      for (int k = 0; k < p.nKeys; k++) {
        uint8_t a = p.keyA[k], b = p.keyB[k];
        if (side[a] >= 0 && side[b] < 0) { side[b] = !side[a]; progress = true; }
        else if (side[b] >= 0 && side[a] < 0) { side[a] = !side[b]; progress = true; }
        else if (side[a] >= 0 && side[a] == side[b]) return false;
      }
    } while (progress);
    int k = 0;
    while (k < p.nKeys && side[p.keyA[k]] >= 0) k++;
    if (k == p.nKeys) break;
    side[p.keyA[k]] = 0;           // seed the next unconnected component
  }
  rowMask = colMask = 0;
  for (int i = 0; i < MAX_PINS; i++) {
    if (side[i] == 0) rowMask |= 1u << i;
    if (side[i] == 1) colMask |= 1u << i;
  }
  return true;
}

// Length of the run of keys at the start that all share one pin.
static int leading_run(const Profile& p) {
  int best = 1;
  for (uint8_t pin : {p.keyA[0], p.keyB[0]}) {
    int run = 1;
    while (run < p.nKeys && (p.keyA[run] == pin || p.keyB[run] == pin)) run++;
    if (run > best) best = run;
  }
  return best;
}

void profile_finalize_learned(Profile& p) {
  uint8_t maxPin = 0;
  for (int k = 0; k < p.nKeys; k++) maxPin = max(maxPin, max(p.keyA[k], p.keyB[k]));
  p.nPins = p.nKeys ? maxPin + 1 : 0;

  uint8_t common, rows, cols;
  if (detect_common(p, common)) {
    p.topo = TOPO_COMMON;
    p.gridCols = p.nKeys <= 7 ? p.nKeys : auto_grid_cols(p.nKeys);
  } else if (p.nKeys >= 4 && detect_matrix(p, rows, cols) &&
             __builtin_popcount(rows) > 1 && __builtin_popcount(cols) > 1) {
    p.topo = TOPO_MATRIX;
    // Keep the learned (physical) order. If the user pressed row by row, the
    // first run of keys sharing a pin is one grid row.
    int nr = __builtin_popcount(rows), nc = __builtin_popcount(cols);
    int run = leading_run(p);
    p.gridCols = (run == nr || run == nc) ? run : auto_grid_cols(p.nKeys);
  } else {
    p.topo = TOPO_CUSTOM;
    p.gridCols = auto_grid_cols(p.nKeys);
  }
  if (!p.gridCols) p.gridCols = 1;
}

void profile_describe(const Profile& p, char* buf, size_t len) {
  uint8_t c, r, cl;
  switch (p.topo) {
    case TOPO_COMMON:
      if (detect_common(p, c))
        snprintf(buf, len, "%u-pin, common on pin %u, %u keys", p.nPins, c + 1, p.nKeys);
      else
        snprintf(buf, len, "%u-pin common, %u keys", p.nPins, p.nKeys);
      break;
    case TOPO_MATRIX:
      if (detect_matrix(p, r, cl))
        snprintf(buf, len, "%u-pin, %ux%u matrix, %u keys", p.nPins,
                 __builtin_popcount(r), __builtin_popcount(cl), p.nKeys);
      else
        snprintf(buf, len, "%u-pin matrix, %u keys", p.nPins, p.nKeys);
      break;
    default:
      snprintf(buf, len, "%u-pin custom, %u keys", p.nPins, p.nKeys);
  }
}

// ---------------- NVS ----------------
static Preferences prefs;
static const char* NS = "mtester";

int profiles_count() {
  prefs.begin(NS, true);
  int n = prefs.getUChar("n", 0);
  prefs.end();
  return n;
}

bool profiles_load(int idx, Profile& p) {
  char key[12];
  snprintf(key, sizeof(key), "p%d", idx);
  prefs.begin(NS, true);
  size_t got = prefs.getBytes(key, &p, sizeof(Profile));
  prefs.end();
  p.name[sizeof(p.name) - 1] = 0;
  return got == sizeof(Profile);
}

bool profiles_save(const Profile& p) {
  int n = profiles_count();
  int slot = n;
  Profile tmp;
  for (int i = 0; i < n; i++)
    if (profiles_load(i, tmp) && strcmp(tmp.name, p.name) == 0) { slot = i; break; }
  if (slot >= MAX_PROFILES) return false;
  char key[12];
  snprintf(key, sizeof(key), "p%d", slot);
  prefs.begin(NS, false);
  bool ok = prefs.putBytes(key, &p, sizeof(Profile)) == sizeof(Profile);
  if (ok && slot == n) prefs.putUChar("n", n + 1);
  prefs.end();
  return ok;
}

bool profiles_delete(int idx) {
  int n = profiles_count();
  if (idx < 0 || idx >= n) return false;
  Profile tmp;
  char key[12];
  prefs.begin(NS, false);
  for (int i = idx; i < n - 1; i++) {
    snprintf(key, sizeof(key), "p%d", i + 1);
    prefs.getBytes(key, &tmp, sizeof(tmp));
    snprintf(key, sizeof(key), "p%d", i);
    prefs.putBytes(key, &tmp, sizeof(tmp));
  }
  snprintf(key, sizeof(key), "p%d", n - 1);
  prefs.remove(key);
  prefs.putUChar("n", n - 1);
  prefs.end();
  return true;
}

// ============================================================================
//  Test session
// ============================================================================
void session_start(TestSession& t, const Profile& p, const ConnState& now) {
  memset(&t, 0, sizeof(t));
  t.prof = p;
  for (int k = 0; k < p.nKeys; k++) {
    bool on = now.connected(p.keyA[k], p.keyB[k]);
    t.active[k] = on;
    t.stuck[k] = on;
    t.armed[k] = !on;
  }
  t.idleDirty = now.any();
  session_update(t, now);   // record any unexpected idle connections as faults
}

static void add_fault(TestSession& t, uint8_t a, uint8_t b, uint32_t& ev) {
  if (a > b) { uint8_t x = a; a = b; b = x; }
  for (int i = 0; i < t.nFaults; i++)
    if (t.faultA[i] == a && t.faultB[i] == b) return;
  if (t.nFaults < MAX_FAULTS) {
    t.faultA[t.nFaults] = a;
    t.faultB[t.nFaults] = b;
    t.nFaults++;
    ev |= EV_FAULT;
  }
}

uint32_t session_update(TestSession& t, const ConnState& s) {
  uint32_t ev = 0;
  const Profile& p = t.prof;
  bool wasComplete = session_pass(t);

  // Union-find over pins using the expected keys that are currently closed.
  uint8_t root[MAX_PINS];
  for (uint8_t i = 0; i < MAX_PINS; i++) root[i] = i;
  auto find = [&](uint8_t x) { while (root[x] != x) x = root[x] = root[root[x]]; return x; };

  // Only credit a key when it is the only one closed: with several keys held,
  // a matrix can show "ghost" keys that aren't really pressed.
  int closed = 0;
  for (int k = 0; k < p.nKeys; k++) closed += s.connected(p.keyA[k], p.keyB[k]);

  for (int k = 0; k < p.nKeys; k++) {
    bool on = s.connected(p.keyA[k], p.keyB[k]);
    if (on) root[find(p.keyA[k])] = find(p.keyB[k]);

    if (on && !t.active[k]) {
      if (t.armed[k] && closed == 1) {
        t.presses[k]++;
        if (!t.tested[k]) { t.tested[k] = true; t.testedCount++; }
      }
      ev |= EV_PRESS;
    } else if (!on && t.active[k]) {
      t.armed[k] = true;          // stuck key released -> now testable
      ev |= EV_RELEASE;
    }
    t.active[k] = on;
  }

  // Any connection that the closed keys can't explain is a fault
  // (short between traces, wrong pin, crosstalk).
  for (uint8_t i = 0; i < MAX_PINS; i++)
    for (uint8_t j = i + 1; j < MAX_PINS; j++)
      if (s.connected(i, j) && find(i) != find(j)) add_fault(t, i, j, ev);

  if (s.leak & ~t.leak) { t.leak |= s.leak; ev |= EV_FAULT; }

  if (!wasComplete && session_pass(t)) ev |= EV_COMPLETE;
  return ev;
}

bool session_has_faults(const TestSession& t) {
  if (t.nFaults || t.leak) return true;
  for (int k = 0; k < t.prof.nKeys; k++) if (t.stuck[k] && !t.armed[k]) return true;
  return false;
}

bool session_pass(const TestSession& t) {
  return t.prof.nKeys > 0 && t.testedCount == t.prof.nKeys && !session_has_faults(t);
}

void session_fault_text(const TestSession& t, char* buf, size_t len) {
  size_t o = 0;
  buf[0] = 0;
  auto put = [&](const char* fmt, auto... args) {
    if (o < len) o += snprintf(buf + o, len - o, fmt, args...);
  };
  for (int i = 0; i < t.nFaults; i++)
    put("%sShort %u-%u", o ? ", " : "", t.faultA[i] + 1, t.faultB[i] + 1);
  for (int k = 0; k < t.prof.nKeys; k++)
    if (t.stuck[k] && !t.armed[k]) put("%sKey %d stuck", o ? ", " : "", k + 1);
  for (int i = 0; i < MAX_PINS; i++)
    if (t.leak & (1u << i)) put("%sPin %d to GND", o ? ", " : "", i + 1);
}

// ============================================================================
//  Learn session
// ============================================================================
void learn_start(LearnSession& l) {
  memset(&l, 0, sizeof(l));
  strcpy(l.prof.name, "New switch");
  l.waitRelease = true;   // require a clean idle state before the first key
  l.lastDup = -1;
}

LearnEvent learn_update(LearnSession& l, const ConnState& s) {
  if (!s.any()) { l.waitRelease = false; return LE_NONE; }
  if (l.waitRelease) return LE_NONE;
  l.waitRelease = true;  // one event per press

  int8_t g[MAX_PINS];
  int groups = conn_groups(s, g);
  int members = 0;
  uint8_t a = 0xFF, b = 0xFF;
  for (uint8_t i = 0; i < MAX_PINS; i++) {
    if (g[i] < 0) continue;
    members++;
    if (a == 0xFF) a = i; else b = i;
  }
  if (groups != 1 || members != 2 || s.leak) return LE_MULTI;

  int dup = profile_find_key(l.prof, a, b);
  if (dup >= 0) { l.lastDup = dup; return LE_DUPLICATE; }
  if (l.prof.nKeys >= MAX_KEYS) return LE_FULL;

  l.prof.keyA[l.prof.nKeys] = a;
  l.prof.keyB[l.prof.nKeys] = b;
  l.prof.nKeys++;
  profile_finalize_learned(l.prof);
  return LE_ADDED;
}

void learn_undo(LearnSession& l) {
  if (l.prof.nKeys) l.prof.nKeys--;
  profile_finalize_learned(l.prof);
}

// ============================================================================
//  Profile helpers
// ============================================================================
void profiles_seed_defaults() {
  if (profiles_count() > 0) return;
  Profile p;
  profile_make_common(p, 4, 0); profiles_save(p);
  profile_make_common(p, 5, 0); profiles_save(p);
  profile_make_common(p, 8, 0); profiles_save(p);
  profile_make_matrix(p, 7, 3); profiles_save(p);   // 3x4 keypad
  profile_make_matrix(p, 8, 4); profiles_save(p);   // 4x4 keypad
}

bool profiles_find(const char* name, Profile& p) {
  int n = profiles_count();
  for (int i = 0; i < n; i++)
    if (profiles_load(i, p) && strcmp(p.name, name) == 0) return true;
  return false;
}

void profiles_set_last(const char* name) {
  prefs.begin(NS, false);
  prefs.putString("last", name);
  prefs.end();
}

bool profiles_get_last(Profile& p) {
  prefs.begin(NS, true);
  String last = prefs.getString("last", "");
  prefs.end();
  return last.length() && profiles_find(last.c_str(), p);
}

void conn_text(const ConnState& s, char* buf, size_t len, const char* sep) {
  int8_t g[MAX_PINS];
  int n = conn_groups(s, g);
  size_t o = 0;
  buf[0] = 0;
  for (int k = 0; k < n && o < len; k++) {
    if (k) o += snprintf(buf + o, len - o, "%s", sep);
    bool first = true;
    for (int i = 0; i < MAX_PINS && o < len; i++)
      if (g[i] == k) { o += snprintf(buf + o, len - o, first ? "%d" : "-%d", i + 1); first = false; }
  }
  if (s.leak && o < len) {
    o += snprintf(buf + o, len - o, "%sGND:", o ? sep : "");
    for (int i = 0; i < MAX_PINS && o < len; i++)
      if (s.leak & (1u << i)) o += snprintf(buf + o, len - o, " %d", i + 1);
  }
}

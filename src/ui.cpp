// LVGL screens for the membrane switch tester.
//
//  Home ──┬─ Test      (grid of keys, PASS/FAIL, unit counter)
//         ├─ Learn     (press keys in order -> auto-detect layout -> save)
//         ├─ Profiles  (saved layouts; + template builder)
//         └─ Monitor   (live pin-to-pin connections, for unknown parts)

#include <lvgl.h>
#include <stdio.h>
#include <string.h>
#include "ui.h"
#include "version.h"

#define COL_BG      lv_color_hex(0x11161C)
#define COL_PANEL   lv_color_hex(0x1C242D)
#define COL_HEADER  lv_color_hex(0x0A0E12)
#define COL_TEXT    lv_color_hex(0xE6EDF3)
#define COL_MUTED   lv_color_hex(0x8B98A5)
#define COL_IDLE    lv_color_hex(0x2A333D)
#define COL_ACCENT  lv_color_hex(0x3BA0FF)
#define COL_GREEN   lv_color_hex(0x2EB872)
#define COL_RED     lv_color_hex(0xE5484D)
#define COL_AMBER   lv_color_hex(0xF5A623)

#define HEADER_H 34

typedef void (*BuildFn)();
enum ScreenId { SCR_HOME, SCR_PROFILES, SCR_NEW, SCR_LEARN, SCR_TEST, SCR_MONITOR };

static ScreenId g_scr = SCR_HOME;
static Profile g_prof;
static bool g_hasProf = false;
static TestSession g_test;
static LearnSession g_learn;
static uint32_t g_units = 0, g_unitsPass = 0;

static lv_obj_t* g_status = nullptr;
static int g_statusCache = -2;

static void build_home();
static void build_profiles();
static void build_new();
static void build_learn();
static void build_test();
static void build_monitor();

// ============================================================================
//  Small helpers
// ============================================================================
static void nav_cb(void* p) { ((BuildFn)p)(); }
// Screens are rebuilt from an async call so we never delete the screen whose
// button event is still being processed.
static void nav(BuildFn fn) { lv_async_call(nav_cb, (void*)fn); }

static void load(lv_obj_t* scr) {
  lv_screen_load_anim(scr, LV_SCR_LOAD_ANIM_NONE, 0, 0, true);
}

static lv_obj_t* make_screen() {
  lv_obj_t* s = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(s, COL_BG, 0);
  lv_obj_set_style_bg_opa(s, LV_OPA_COVER, 0);
  lv_obj_remove_flag(s, LV_OBJ_FLAG_SCROLLABLE);
  return s;
}

static lv_obj_t* label(lv_obj_t* parent, const char* txt, const lv_font_t* font, lv_color_t col) {
  lv_obj_t* l = lv_label_create(parent);
  lv_label_set_text(l, txt);
  lv_obj_set_style_text_font(l, font, 0);
  lv_obj_set_style_text_color(l, col, 0);
  return l;
}

static lv_obj_t* plain(lv_obj_t* parent) {
  lv_obj_t* o = lv_obj_create(parent);
  lv_obj_remove_style_all(o);
  lv_obj_remove_flag(o, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_remove_flag(o, LV_OBJ_FLAG_CLICKABLE);
  return o;
}

static lv_obj_t* button(lv_obj_t* parent, const char* txt, lv_color_t bg,
                        lv_event_cb_t cb, void* ud, int w, int h) {
  lv_obj_t* b = lv_button_create(parent);
  lv_obj_set_size(b, w, h);
  lv_obj_set_style_bg_color(b, bg, 0);
  lv_obj_set_style_radius(b, 8, 0);
  lv_obj_set_style_shadow_width(b, 0, 0);
  if (txt) {
    lv_obj_t* l = label(b, txt, &lv_font_montserrat_16, COL_TEXT);
    lv_obj_center(l);
  }
  if (cb) lv_obj_add_event_cb(b, cb, LV_EVENT_CLICKED, ud);
  return b;
}

static void set_enabled(lv_obj_t* o, bool en) {
  if (en) lv_obj_remove_state(o, LV_STATE_DISABLED);
  else lv_obj_add_state(o, LV_STATE_DISABLED);
}

static void update_status() {
  if (!g_status) return;
  int st = scanner_ok() ? scanner_address() : -1;
  if (st == g_statusCache) return;
  g_statusCache = st;
  if (st >= 0) {
    lv_label_set_text_fmt(g_status, LV_SYMBOL_OK " PCF 0x%02X", st);
    lv_obj_set_style_text_color(g_status, COL_GREEN, 0);
  } else {
    lv_label_set_text(g_status, LV_SYMBOL_WARNING " No PCF8574");
    lv_obj_set_style_text_color(g_status, COL_RED, 0);
  }
}

static void back_cb(lv_event_t* e) { nav((BuildFn)lv_event_get_user_data(e)); }

static lv_obj_t* header(lv_obj_t* scr, const char* title, BuildFn backTo) {
  lv_obj_t* h = plain(scr);
  lv_obj_set_size(h, SCREEN_W, HEADER_H);
  lv_obj_set_style_bg_color(h, COL_HEADER, 0);
  lv_obj_set_style_bg_opa(h, LV_OPA_COVER, 0);

  int x = 10;
  if (backTo) {
    lv_obj_t* b = button(h, LV_SYMBOL_LEFT, COL_HEADER, back_cb, (void*)backTo, 44, HEADER_H);
    lv_obj_set_style_radius(b, 0, 0);
    x = 48;
  }
  lv_obj_t* t = label(h, title, &lv_font_montserrat_16, COL_TEXT);
  lv_label_set_long_mode(t, LV_LABEL_LONG_DOT);
  lv_obj_set_width(t, SCREEN_W - x - 108);
  lv_obj_align(t, LV_ALIGN_LEFT_MID, x, 0);

  g_status = label(h, "", &lv_font_montserrat_12, COL_MUTED);
  lv_obj_align(g_status, LV_ALIGN_RIGHT_MID, -8, 0);
  g_statusCache = -2;
  update_status();
  return h;
}

static void toast(const char* msg, lv_color_t bg) {
  lv_obj_t* t = lv_label_create(lv_layer_top());
  lv_label_set_text(t, msg);
  lv_obj_set_style_text_color(t, COL_TEXT, 0);
  lv_obj_set_style_bg_color(t, bg, 0);
  lv_obj_set_style_bg_opa(t, LV_OPA_COVER, 0);
  lv_obj_set_style_pad_all(t, 8, 0);
  lv_obj_set_style_radius(t, 6, 0);
  lv_obj_align(t, LV_ALIGN_BOTTOM_MID, 0, -50);
  lv_obj_delete_delayed(t, 1800);
}

// Dimmed full-screen backdrop with a centred panel. Returns the panel.
static lv_obj_t* modal(int w, int h) {
  lv_obj_t* bg = plain(lv_screen_active());
  lv_obj_set_size(bg, SCREEN_W, SCREEN_H);
  lv_obj_set_style_bg_color(bg, lv_color_black(), 0);
  lv_obj_set_style_bg_opa(bg, LV_OPA_60, 0);
  lv_obj_add_flag(bg, LV_OBJ_FLAG_CLICKABLE);   // swallow taps behind the panel

  lv_obj_t* p = lv_obj_create(bg);
  lv_obj_set_size(p, w, h);
  lv_obj_center(p);
  lv_obj_set_style_bg_color(p, COL_PANEL, 0);
  lv_obj_set_style_border_width(p, 0, 0);
  lv_obj_set_style_radius(p, 10, 0);
  lv_obj_set_style_pad_all(p, 10, 0);
  lv_obj_remove_flag(p, LV_OBJ_FLAG_SCROLLABLE);
  return p;
}

// Delete the top-level overlay that contains `o`.
static void close_overlay(lv_obj_t* o) {
  lv_obj_t* scr = lv_screen_active();
  while (lv_obj_get_parent(o) && lv_obj_get_parent(o) != scr) o = lv_obj_get_parent(o);
  lv_obj_delete_async(o);
}

// ---------------------------------------------------------------- name prompt
typedef void (*NameDoneFn)(const char*);
static NameDoneFn g_nameDone = nullptr;

static void name_kb_cb(lv_event_t* e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code != LV_EVENT_READY && code != LV_EVENT_CANCEL) return;
  lv_obj_t* kb = (lv_obj_t*)lv_event_get_target(e);
  lv_obj_t* ta = lv_keyboard_get_textarea(kb);
  if (code == LV_EVENT_READY && g_nameDone) {
    const char* txt = lv_textarea_get_text(ta);
    if (txt && txt[0]) g_nameDone(txt);
  }
  close_overlay(kb);
}

static void prompt_name(const char* title, const char* initial, NameDoneFn done) {
  g_nameDone = done;
  lv_obj_t* root = plain(lv_screen_active());
  lv_obj_set_size(root, SCREEN_W, SCREEN_H);
  lv_obj_set_style_bg_color(root, COL_BG, 0);
  lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);
  lv_obj_add_flag(root, LV_OBJ_FLAG_CLICKABLE);

  lv_obj_t* t = label(root, title, &lv_font_montserrat_14, COL_MUTED);
  lv_obj_set_pos(t, 8, 6);

  lv_obj_t* ta = lv_textarea_create(root);
  lv_textarea_set_one_line(ta, true);
  lv_textarea_set_max_length(ta, sizeof(((Profile*)0)->name) - 1);
  lv_textarea_set_text(ta, initial);
  lv_obj_set_width(ta, SCREEN_W - 16);
  lv_obj_set_pos(ta, 8, 26);
  lv_obj_add_state(ta, LV_STATE_FOCUSED);

  lv_obj_t* kb = lv_keyboard_create(root);
  lv_obj_set_size(kb, SCREEN_W, 166);
  lv_obj_align(kb, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_keyboard_set_textarea(kb, ta);
  lv_obj_add_event_cb(kb, name_kb_cb, LV_EVENT_ALL, nullptr);
}

// ============================================================================
//  Home
// ============================================================================
static void home_tile(lv_obj_t* scr, int x, int y, const char* sym, const char* title,
                      const char* sub, lv_color_t col, lv_event_cb_t cb) {
  lv_obj_t* b = button(scr, nullptr, COL_PANEL, cb, nullptr, 148, 84);
  lv_obj_set_pos(b, x, y);
  lv_obj_set_style_pad_all(b, 8, 0);
  lv_obj_set_style_border_side(b, LV_BORDER_SIDE_LEFT, 0);
  lv_obj_set_style_border_width(b, 4, 0);
  lv_obj_set_style_border_color(b, col, 0);

  lv_obj_t* s = label(b, sym, &lv_font_montserrat_22, col);
  lv_obj_align(s, LV_ALIGN_TOP_LEFT, 2, 0);
  lv_obj_t* t = label(b, title, &lv_font_montserrat_16, COL_TEXT);
  lv_obj_align(t, LV_ALIGN_TOP_LEFT, 2, 30);
  lv_obj_t* d = label(b, sub, &lv_font_montserrat_12, COL_MUTED);
  lv_label_set_long_mode(d, LV_LABEL_LONG_DOT);
  lv_obj_set_width(d, 128);
  lv_obj_align(d, LV_ALIGN_TOP_LEFT, 2, 52);
}

static void build_home() {
  g_scr = SCR_HOME;
  fb_led(false, false, false);
  lv_obj_t* scr = make_screen();
  char title[32];
  snprintf(title, sizeof(title), "Membrane Tester v%s", APP_VERSION);
  header(scr, title, nullptr);

  home_tile(scr, 8, 42, LV_SYMBOL_PLAY, "Test",
            g_hasProf ? g_prof.name : "Pick a profile", COL_GREEN,
            [](lv_event_t*) { nav(g_hasProf ? build_test : build_profiles); });
  home_tile(scr, 164, 42, LV_SYMBOL_EDIT, "Learn", "Press keys to map", COL_ACCENT,
            [](lv_event_t*) { nav(build_learn); });
  int nProf = profiles_count();
  char profSub[32];
  if (nProf == 0) snprintf(profSub, sizeof(profSub), "None saved yet");
  else snprintf(profSub, sizeof(profSub), "%d saved layout%s", nProf, nProf == 1 ? "" : "s");
  home_tile(scr, 8, 134, LV_SYMBOL_LIST, "Profiles", profSub, COL_AMBER,
            [](lv_event_t*) { nav(build_profiles); });
  home_tile(scr, 164, 134, LV_SYMBOL_EYE_OPEN, "Pin monitor", "Live connections",
            lv_color_hex(0xA371F7), [](lv_event_t*) { nav(build_monitor); });

  lv_obj_t* f = label(scr, "", &lv_font_montserrat_12, COL_MUTED);
  lv_label_set_text_fmt(f, "Units this session: %lu    Pass %lu    Fail %lu",
                        (unsigned long)g_units, (unsigned long)g_unitsPass,
                        (unsigned long)(g_units - g_unitsPass));
  lv_obj_align(f, LV_ALIGN_BOTTOM_MID, 0, -4);
  load(scr);
}

// ============================================================================
//  Profiles
// ============================================================================
static int g_rowIdx = -1;

static void use_profile(const Profile& p) {
  g_prof = p;
  g_hasProf = true;
  profiles_set_last(p.name);
}

static void row_test_cb(lv_event_t*) {
  Profile p;
  if (profiles_load(g_rowIdx, p)) { use_profile(p); nav(build_test); }
}

static void row_delete_cb(lv_event_t* e) {
  lv_obj_t* b = (lv_obj_t*)lv_event_get_target(e);
  if (!lv_obj_has_state(b, LV_STATE_CHECKED)) {   // first tap arms, second deletes
    lv_obj_add_state(b, LV_STATE_CHECKED);
    lv_label_set_text(lv_obj_get_child(b, 0), "Sure?");
    return;
  }
  Profile p;
  if (profiles_load(g_rowIdx, p) && g_hasProf && strcmp(p.name, g_prof.name) == 0)
    g_hasProf = false;
  profiles_delete(g_rowIdx);
  nav(build_profiles);
}

static void row_cb(lv_event_t* e) {
  g_rowIdx = (int)(intptr_t)lv_event_get_user_data(e);
  Profile p;
  if (!profiles_load(g_rowIdx, p)) return;
  char desc[64];
  profile_describe(p, desc, sizeof(desc));

  lv_obj_t* m = modal(290, 150);
  lv_obj_t* t = label(m, p.name, &lv_font_montserrat_16, COL_TEXT);
  lv_obj_align(t, LV_ALIGN_TOP_LEFT, 0, 0);
  lv_obj_t* d = label(m, desc, &lv_font_montserrat_12, COL_MUTED);
  lv_obj_set_width(d, 266);
  lv_obj_align(d, LV_ALIGN_TOP_LEFT, 0, 26);

  lv_obj_t* b1 = button(m, LV_SYMBOL_PLAY " Test", COL_GREEN, row_test_cb, nullptr, 84, 38);
  lv_obj_align(b1, LV_ALIGN_BOTTOM_LEFT, 0, 0);
  lv_obj_t* b2 = button(m, "Delete", COL_RED, row_delete_cb, nullptr, 84, 38);
  lv_obj_align(b2, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_t* b3 = button(m, "Cancel", COL_IDLE,
                        [](lv_event_t* e) { close_overlay((lv_obj_t*)lv_event_get_target(e)); },
                        nullptr, 84, 38);
  lv_obj_align(b3, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
}

static void build_profiles() {
  g_scr = SCR_PROFILES;
  lv_obj_t* scr = make_screen();
  int n = profiles_count();
  char title[32];
  snprintf(title, sizeof(title), "Profiles (%d/%d)", n, MAX_PROFILES);
  header(scr, title, build_home);

  lv_obj_t* list = plain(scr);
  lv_obj_add_flag(list, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_scroll_dir(list, LV_DIR_VER);
  lv_obj_set_pos(list, 0, HEADER_H);
  lv_obj_set_size(list, SCREEN_W, 160);
  lv_obj_set_style_pad_all(list, 6, 0);
  lv_obj_set_style_pad_row(list, 4, 0);
  lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);

  if (n == 0) {
    lv_obj_t* l = label(list, "No saved profiles.\nUse Learn to map a switch by pressing its keys, "
                              "or + Template for a standard layout.",
                        &lv_font_montserrat_14, COL_MUTED);
    lv_obj_set_width(l, 300);
  }
  for (int i = 0; i < n; i++) {
    Profile p;
    if (!profiles_load(i, p)) continue;
    char desc[64];
    profile_describe(p, desc, sizeof(desc));

    lv_obj_t* r = button(list, nullptr, COL_PANEL, row_cb, (void*)(intptr_t)i, lv_pct(100), 44);
    lv_obj_set_style_pad_hor(r, 10, 0);
    lv_obj_set_style_pad_ver(r, 4, 0);
    if (g_hasProf && strcmp(p.name, g_prof.name) == 0) {
      lv_obj_set_style_border_width(r, 2, 0);
      lv_obj_set_style_border_color(r, COL_ACCENT, 0);
    }
    lv_obj_t* nm = label(r, p.name, &lv_font_montserrat_14, COL_TEXT);
    lv_obj_align(nm, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_t* ds = label(r, desc, &lv_font_montserrat_12, COL_MUTED);
    lv_obj_align(ds, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_t* ar = label(r, LV_SYMBOL_RIGHT, &lv_font_montserrat_14, COL_MUTED);
    lv_obj_align(ar, LV_ALIGN_RIGHT_MID, 0, 0);
  }

  lv_obj_t* b1 = button(scr, LV_SYMBOL_PLUS " Template", COL_IDLE,
                        [](lv_event_t*) { nav(build_new); }, nullptr, 150, 38);
  lv_obj_set_pos(b1, 6, 198);
  lv_obj_t* b2 = button(scr, LV_SYMBOL_EDIT " Learn", COL_ACCENT,
                        [](lv_event_t*) { nav(build_learn); }, nullptr, 150, 38);
  lv_obj_set_pos(b2, 164, 198);
  load(scr);
}

// ============================================================================
//  New profile from a template (common contact / matrix)
// ============================================================================
static uint8_t n_pins = 4, n_layout = 0, n_param = 0;
static char n_name[sizeof(((Profile*)0)->name)];
static bool n_nameEdited = false;
static Profile n_prof;
static lv_obj_t *n_ddParam, *n_paramLbl, *n_preview, *n_nameLbl;

static void new_update_preview() {
  if (n_layout == 0) profile_make_common(n_prof, n_pins, n_param);
  else profile_make_matrix(n_prof, n_pins, n_param + 1);
  if (!n_nameEdited) strlcpy(n_name, n_prof.name, sizeof(n_name));
  strlcpy(n_prof.name, n_name, sizeof(n_prof.name));

  char buf[128], desc[64];
  profile_describe(n_prof, desc, sizeof(desc));
  if (n_layout == 0)
    snprintf(buf, sizeof(buf), "%s\nPin %u is shared by every key.", desc, n_param + 1);
  else
    snprintf(buf, sizeof(buf), "%s\nRows = pins 1-%u, columns = pins %u-%u.", desc,
             n_param + 1, n_param + 2, n_pins);
  lv_label_set_text(n_preview, buf);
  lv_label_set_text_fmt(n_nameLbl, "%s  " LV_SYMBOL_EDIT, n_name);
}

static void new_rebuild_param() {
  char opts[200] = "";
  size_t o = 0;
  if (n_layout == 0) {
    lv_label_set_text(n_paramLbl, "Common pin");
    if (n_param >= n_pins) n_param = 0;
    for (int i = 1; i <= n_pins; i++) o += snprintf(opts + o, sizeof(opts) - o, "%sPin %d", i > 1 ? "\n" : "", i);
  } else {
    lv_label_set_text(n_paramLbl, "Rows");
    if (n_param >= n_pins - 1) n_param = n_pins / 2 - 1;
    for (int r = 1; r < n_pins; r++)
      o += snprintf(opts + o, sizeof(opts) - o, "%s%d x %d", r > 1 ? "\n" : "", r, n_pins - r);
  }
  lv_dropdown_set_options(n_ddParam, opts);
  lv_dropdown_set_selected(n_ddParam, n_param);
  new_update_preview();
}

static lv_obj_t* form_row(lv_obj_t* parent, const char* text, lv_obj_t** lblOut = nullptr) {
  lv_obj_t* r = plain(parent);
  lv_obj_set_size(r, lv_pct(100), LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(r, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(r, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_t* l = label(r, text, &lv_font_montserrat_14, COL_MUTED);
  lv_obj_set_width(l, 104);
  if (lblOut) *lblOut = l;
  return r;
}

static lv_obj_t* form_dropdown(lv_obj_t* row, const char* opts, uint32_t sel, lv_event_cb_t cb) {
  lv_obj_t* dd = lv_dropdown_create(row);
  lv_dropdown_set_options(dd, opts);
  lv_dropdown_set_selected(dd, sel);
  lv_obj_set_flex_grow(dd, 1);
  lv_obj_set_height(dd, 34);
  lv_obj_set_style_pad_ver(dd, 7, 0);
  lv_obj_add_event_cb(dd, cb, LV_EVENT_VALUE_CHANGED, nullptr);
  return dd;
}

static void new_save(bool thenTest) {
  if (!profiles_save(n_prof)) { toast("Profile storage full", COL_RED); return; }
  use_profile(n_prof);
  toast(LV_SYMBOL_OK " Saved", COL_GREEN);
  nav(thenTest ? build_test : build_profiles);
}

static void build_new() {
  g_scr = SCR_NEW;
  n_nameEdited = false;
  lv_obj_t* scr = make_screen();
  header(scr, "New from template", build_profiles);

  lv_obj_t* body = plain(scr);
  lv_obj_add_flag(body, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_pos(body, 0, HEADER_H);
  lv_obj_set_size(body, SCREEN_W, 160);
  lv_obj_set_style_pad_all(body, 6, 0);
  lv_obj_set_style_pad_row(body, 4, 0);
  lv_obj_set_flex_flow(body, LV_FLEX_FLOW_COLUMN);

  form_dropdown(form_row(body, "Pins"), "4\n5\n6\n7\n8", n_pins - 4, [](lv_event_t* e) {
    n_pins = 4 + lv_dropdown_get_selected((lv_obj_t*)lv_event_get_target(e));
    new_rebuild_param();
  });
  form_dropdown(form_row(body, "Layout"), "Common contact\nMatrix", n_layout, [](lv_event_t* e) {
    n_layout = lv_dropdown_get_selected((lv_obj_t*)lv_event_get_target(e));
    n_param = n_layout ? n_pins / 2 - 1 : 0;
    new_rebuild_param();
  });
  n_ddParam = form_dropdown(form_row(body, "Common pin", &n_paramLbl), "Pin 1", 0, [](lv_event_t* e) {
    n_param = lv_dropdown_get_selected((lv_obj_t*)lv_event_get_target(e));
    new_update_preview();
  });

  lv_obj_t* nr = form_row(body, "Name");
  lv_obj_t* nb = button(nr, nullptr, COL_PANEL, [](lv_event_t*) {
    prompt_name("Profile name", n_name, [](const char* s) {
      strlcpy(n_name, s, sizeof(n_name));
      n_nameEdited = true;
      new_update_preview();
    });
  }, nullptr, 10, 34);
  lv_obj_set_flex_grow(nb, 1);
  n_nameLbl = label(nb, "", &lv_font_montserrat_14, COL_TEXT);
  lv_label_set_long_mode(n_nameLbl, LV_LABEL_LONG_DOT);
  lv_obj_set_width(n_nameLbl, lv_pct(100));
  lv_obj_align(n_nameLbl, LV_ALIGN_LEFT_MID, 0, 0);

  n_preview = label(body, "", &lv_font_montserrat_12, COL_MUTED);
  lv_obj_set_width(n_preview, lv_pct(100));

  lv_obj_t* b1 = button(scr, LV_SYMBOL_SAVE " Save", COL_IDLE,
                        [](lv_event_t*) { new_save(false); }, nullptr, 150, 38);
  lv_obj_set_pos(b1, 6, 198);
  lv_obj_t* b2 = button(scr, LV_SYMBOL_PLAY " Save & test", COL_GREEN,
                        [](lv_event_t*) { new_save(true); }, nullptr, 150, 38);
  lv_obj_set_pos(b2, 164, 198);

  new_rebuild_param();
  load(scr);
}

// ============================================================================
//  Learn: press each key once -> profile
// ============================================================================
static lv_obj_t *l_count, *l_layout, *l_event, *l_live, *l_undo, *l_save;

static void learn_refresh() {
  lv_label_set_text_fmt(l_count, "%u key%s", g_learn.prof.nKeys, g_learn.prof.nKeys == 1 ? "" : "s");
  if (g_learn.prof.nKeys) {
    char d[64];
    profile_describe(g_learn.prof, d, sizeof(d));
    lv_label_set_text_fmt(l_layout, "Detected: %s", d);
  } else {
    lv_label_set_text(l_layout, "Layout is detected automatically");
  }
  set_enabled(l_undo, g_learn.prof.nKeys > 0);
  set_enabled(l_save, g_learn.prof.nKeys > 0);
}

static void learn_live(const ConnState& s) {
  char buf[64];
  conn_text(s, buf, sizeof(buf), ", ");
  if (buf[0]) lv_label_set_text_fmt(l_live, "Live: pins %s connected", buf);
  else lv_label_set_text(l_live, "Live: nothing pressed");
}

static void learn_event(const char* msg, lv_color_t col) {
  lv_label_set_text(l_event, msg);
  lv_obj_set_style_text_color(l_event, col, 0);
}

static void learn_restart() {
  learn_start(g_learn);
  learn_update(g_learn, scanner_state());   // consume the current (idle) state
  learn_event("Press the first key", COL_TEXT);
  learn_live(scanner_state());
  learn_refresh();
}

static void learn_on_scan(const ConnState& s) {
  char buf[64];
  switch (learn_update(g_learn, s)) {
    case LE_ADDED: {
      int k = g_learn.prof.nKeys - 1;
      snprintf(buf, sizeof(buf), LV_SYMBOL_OK " Key %d = pins %u-%u", k + 1,
               g_learn.prof.keyA[k] + 1, g_learn.prof.keyB[k] + 1);
      learn_event(buf, COL_GREEN);
      fb_beep(2200, 40);
      break;
    }
    case LE_DUPLICATE:
      snprintf(buf, sizeof(buf), "Already learned as key %d", g_learn.lastDup + 1);
      learn_event(buf, COL_AMBER);
      fb_beep(700, 120);
      break;
    case LE_MULTI:
      learn_event(LV_SYMBOL_WARNING " More than 2 pins - one key at a time", COL_RED);
      fb_beep(300, 250);
      break;
    case LE_FULL:
      learn_event("Maximum number of keys reached", COL_RED);
      break;
    default:
      break;
  }
  learn_live(s);
  learn_refresh();
}

static void learn_saved(const char* name) {
  strlcpy(g_learn.prof.name, name, sizeof(g_learn.prof.name));
  if (!profiles_save(g_learn.prof)) { toast("Profile storage full", COL_RED); return; }
  use_profile(g_learn.prof);
  toast(LV_SYMBOL_OK " Saved", COL_GREEN);
  nav(build_test);
}

static void build_learn() {
  g_scr = SCR_LEARN;
  lv_obj_t* scr = make_screen();
  header(scr, "Learn a switch", build_home);

  lv_obj_t* hint = label(scr, "Press each key once, in order (left to right, top to bottom). "
                              "One key at a time.", &lv_font_montserrat_14, COL_MUTED);
  lv_obj_set_width(hint, 300);
  lv_obj_set_pos(hint, 10, 40);

  l_count = label(scr, "", &lv_font_montserrat_28, COL_TEXT);
  lv_obj_set_pos(l_count, 10, 82);
  l_layout = label(scr, "", &lv_font_montserrat_12, COL_MUTED);
  lv_obj_set_pos(l_layout, 10, 116);
  l_event = label(scr, "", &lv_font_montserrat_16, COL_TEXT);
  lv_label_set_long_mode(l_event, LV_LABEL_LONG_DOT);
  lv_obj_set_width(l_event, 300);
  lv_obj_set_pos(l_event, 10, 140);
  l_live = label(scr, "", &lv_font_montserrat_12, COL_MUTED);
  lv_obj_set_pos(l_live, 10, 168);

  l_undo = button(scr, LV_SYMBOL_BACKSPACE " Undo", COL_IDLE, [](lv_event_t*) {
    learn_undo(g_learn);
    learn_event("Removed last key", COL_MUTED);
    learn_refresh();
  }, nullptr, 98, 38);
  lv_obj_set_pos(l_undo, 6, 198);
  lv_obj_t* r = button(scr, LV_SYMBOL_REFRESH " Clear", COL_IDLE,
                       [](lv_event_t*) { learn_restart(); }, nullptr, 98, 38);
  lv_obj_set_pos(r, 111, 198);
  l_save = button(scr, LV_SYMBOL_SAVE " Save", COL_GREEN, [](lv_event_t*) {
    char def[sizeof(g_learn.prof.name)];
    snprintf(def, sizeof(def), "%u-pin %u-key", g_learn.prof.nPins, g_learn.prof.nKeys);
    prompt_name("Name this switch (part number?)", def, learn_saved);
  }, nullptr, 98, 38);
  lv_obj_set_pos(l_save, 216, 198);

  learn_restart();
  load(scr);
}

// ============================================================================
//  Test
// ============================================================================
static lv_obj_t* t_tiles[MAX_KEYS];
static lv_obj_t *t_bar, *t_result, *t_info;

static void test_refresh() {
  const Profile& p = g_test.prof;
  int closed = 0;
  for (int k = 0; k < p.nKeys; k++) closed += g_test.active[k];

  for (int k = 0; k < p.nKeys; k++) {
    lv_color_t c = COL_IDLE;
    if (g_test.active[k]) c = closed > 1 ? COL_AMBER : COL_ACCENT;
    else if (g_test.stuck[k] && !g_test.armed[k]) c = COL_RED;
    else if (g_test.tested[k]) c = COL_GREEN;
    lv_obj_set_style_bg_color(t_tiles[k], c, 0);
  }

  char buf[128], f[96];
  lv_color_t barCol = COL_PANEL, txtCol = COL_TEXT;
  if (session_has_faults(g_test)) {
    session_fault_text(g_test, f, sizeof(f));
    snprintf(buf, sizeof(buf), LV_SYMBOL_CLOSE " FAIL  %s", f);
    barCol = lv_color_hex(0x4A1C1F); txtCol = COL_RED;
  } else if (session_pass(g_test)) {
    snprintf(buf, sizeof(buf), LV_SYMBOL_OK " PASS  all %u keys OK", p.nKeys);
    barCol = lv_color_hex(0x163A28); txtCol = COL_GREEN;
  } else if (!scanner_ok()) {
    snprintf(buf, sizeof(buf), LV_SYMBOL_WARNING " PCF8574 not found");
    txtCol = COL_RED;
  } else if (closed > 1) {
    snprintf(buf, sizeof(buf), "One key at a time");
    txtCol = COL_AMBER;
  } else {
    snprintf(buf, sizeof(buf), "Press every key");
  }
  lv_label_set_text(t_result, buf);
  lv_obj_set_style_text_color(t_result, txtCol, 0);
  lv_obj_set_style_bg_color(t_bar, barCol, 0);

  lv_label_set_text_fmt(t_info, "%u/%u keys    Units %lu  Pass %lu  Fail %lu",
                        g_test.testedCount, p.nKeys, (unsigned long)g_units,
                        (unsigned long)g_unitsPass, (unsigned long)(g_units - g_unitsPass));
}

static void test_restart() {
  session_start(g_test, g_prof, scanner_state());
  fb_led(false, false, false);
  if (session_has_faults(g_test)) { fb_led(true, false, false); fb_beep(300, 350); }
  test_refresh();
}

static void test_next_cb(lv_event_t*) {
  if (g_test.testedCount > 0 || session_has_faults(g_test)) {
    g_units++;
    if (session_pass(g_test)) g_unitsPass++;
  }
  test_restart();
}

static void test_on_scan(const ConnState& s) {
  uint32_t ev = session_update(g_test, s);
  if (ev & EV_FAULT) { fb_beep(300, 350); fb_led(true, false, false); }
  else if (ev & EV_COMPLETE) { fb_beep(2600, 300); fb_led(false, true, false); }
  else if (ev & EV_PRESS) fb_beep(2000, 25);
  test_refresh();
}

static void build_test() {
  g_scr = SCR_TEST;
  lv_obj_t* scr = make_screen();
  header(scr, g_prof.name, build_home);

  // Key grid
  const Profile& p = g_prof;
  int n = p.nKeys ? p.nKeys : 1;
  int cols = p.gridCols ? p.gridCols : auto_grid_cols(n);
  int rows = (n + cols - 1) / cols;
  const int gap = 4, areaX = 4, areaY = HEADER_H + 4, areaW = SCREEN_W - 8, areaH = 154;
  int tw = (areaW - (cols - 1) * gap) / cols;
  int th = (areaH - (rows - 1) * gap) / rows;
  if (th > 70) th = 70;
  int y0 = areaY + (areaH - (rows * th + (rows - 1) * gap)) / 2;
  bool showPins = th >= 38 && tw >= 44;
  const lv_font_t* f = (th >= 30 && tw >= 30) ? &lv_font_montserrat_16 : &lv_font_montserrat_12;

  for (int k = 0; k < p.nKeys; k++) {
    lv_obj_t* t = plain(scr);
    lv_obj_set_size(t, tw, th);
    lv_obj_set_pos(t, areaX + (k % cols) * (tw + gap), y0 + (k / cols) * (th + gap));
    lv_obj_set_style_radius(t, 6, 0);
    lv_obj_set_style_bg_opa(t, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(t, COL_IDLE, 0);
    char num[8];
    snprintf(num, sizeof(num), "%d", k + 1);
    lv_obj_t* l = label(t, num, f, COL_TEXT);
    lv_obj_align(l, LV_ALIGN_CENTER, 0, showPins ? -7 : 0);
    if (showPins) {
      char pins[12];
      snprintf(pins, sizeof(pins), "%u-%u", p.keyA[k] + 1, p.keyB[k] + 1);
      lv_obj_t* pl = label(t, pins, &lv_font_montserrat_12, COL_TEXT);
      lv_obj_set_style_text_opa(pl, LV_OPA_70, 0);
      lv_obj_align(pl, LV_ALIGN_CENTER, 0, 10);
    }
    t_tiles[k] = t;
  }

  // Result bar
  t_bar = plain(scr);
  lv_obj_set_pos(t_bar, 0, 194);
  lv_obj_set_size(t_bar, SCREEN_W, SCREEN_H - 194);
  lv_obj_set_style_bg_opa(t_bar, LV_OPA_COVER, 0);

  t_result = label(t_bar, "", &lv_font_montserrat_16, COL_TEXT);
  lv_label_set_long_mode(t_result, LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_obj_set_width(t_result, 210);
  lv_obj_set_pos(t_result, 8, 5);
  t_info = label(t_bar, "", &lv_font_montserrat_12, COL_MUTED);
  lv_obj_set_pos(t_info, 8, 27);

  lv_obj_t* nb = button(t_bar, LV_SYMBOL_REFRESH " Next", COL_ACCENT, test_next_cb, nullptr, 88, 36);
  lv_obj_align(nb, LV_ALIGN_RIGHT_MID, -5, 0);

  test_restart();
  load(scr);
}

// ============================================================================
//  Pin monitor
// ============================================================================
static lv_obj_t* m_pin[MAX_PINS];
static lv_obj_t *m_text, *m_changes;
static uint32_t m_count = 0;

static void monitor_refresh(const ConnState& s) {
  static const uint32_t groupCols[] = {0x3BA0FF, 0x2EB872, 0xF5A623, 0xA371F7, 0x39C5CF, 0xF778BA};
  int8_t g[MAX_PINS];
  conn_groups(s, g);
  for (int i = 0; i < MAX_PINS; i++) {
    lv_color_t c = COL_IDLE;
    if (s.leak & (1u << i)) c = COL_RED;
    else if (g[i] >= 0) c = lv_color_hex(groupCols[g[i] % 6]);
    lv_obj_set_style_bg_color(m_pin[i], c, 0);
  }
  char buf[96];
  conn_text(s, buf, sizeof(buf), "\n");
  if (!scanner_ok()) lv_label_set_text(m_text, LV_SYMBOL_WARNING " PCF8574 not found");
  else if (buf[0]) lv_label_set_text_fmt(m_text, "Connected:\n%s", buf);
  else lv_label_set_text(m_text, "Nothing connected");
  lv_label_set_text_fmt(m_changes, "State changes: %lu", (unsigned long)m_count);
}

static void build_monitor() {
  g_scr = SCR_MONITOR;
  m_count = 0;
  lv_obj_t* scr = make_screen();
  header(scr, "Pin monitor", build_home);

  lv_obj_t* hint = label(scr, "Pins 1-8 = PCF8574 P0-P7. Press keys to see which pins connect.",
                         &lv_font_montserrat_12, COL_MUTED);
  lv_obj_set_width(hint, 300);
  lv_obj_set_pos(hint, 10, 40);

  const int d = 32, pitch = 38, x0 = (SCREEN_W - (MAX_PINS * pitch - (pitch - d))) / 2;
  for (int i = 0; i < MAX_PINS; i++) {
    lv_obj_t* c = plain(scr);
    lv_obj_set_size(c, d, d);
    lv_obj_set_pos(c, x0 + i * pitch, 76);
    lv_obj_set_style_radius(c, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_opa(c, LV_OPA_COVER, 0);
    char num[4];
    snprintf(num, sizeof(num), "%d", i + 1);
    lv_obj_center(label(c, num, &lv_font_montserrat_14, COL_TEXT));
    m_pin[i] = c;
  }

  m_text = label(scr, "", &lv_font_montserrat_16, COL_TEXT);
  lv_obj_set_pos(m_text, 10, 122);
  m_changes = label(scr, "", &lv_font_montserrat_12, COL_MUTED);
  lv_obj_align(m_changes, LV_ALIGN_BOTTOM_LEFT, 10, -6);

  monitor_refresh(scanner_state());
  load(scr);
}

// ============================================================================
//  Entry points
// ============================================================================
void ui_init() {
  lv_theme_t* th = lv_theme_default_init(lv_display_get_default(), COL_ACCENT, COL_GREEN, true,
                                         &lv_font_montserrat_14);
  lv_display_set_theme(lv_display_get_default(), th);

  g_hasProf = profiles_get_last(g_prof);
  lv_timer_create([](lv_timer_t*) {
    update_status();
    if (g_scr == SCR_MONITOR) monitor_refresh(scanner_state());
  }, 500, nullptr);
  build_home();
}

void ui_on_scan(const ConnState& s) {
  update_status();
  switch (g_scr) {
    case SCR_TEST: test_on_scan(s); break;
    case SCR_LEARN: learn_on_scan(s); break;
    case SCR_MONITOR: m_count++; monitor_refresh(s); break;
    default: break;
  }
}

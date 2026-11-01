# Desktop simulator: real UI + tester logic in an SDL window.
#
#   macOS:  brew install sdl2
#   Linux:  sudo apt install libsdl2-dev
#
#   make            build ./membrane-sim
#   make run        build and run
#
# LVGL source: uses the copy PlatformIO already downloaded for the firmware
# (run `pio run` once first), or set LVGL_DIR to any LVGL 9.2.2 checkout.

PROJ     := ..
LVGL_DIR ?= $(PROJ)/.pio/libdeps/cyd/lvgl
BUILD    := build

SDL_CFLAGS := $(shell sdl2-config --cflags) -I$(shell sdl2-config --prefix)/include
SDL_LIBS   := $(shell sdl2-config --libs)

CPPFLAGS := -DLV_CONF_INCLUDE_SIMPLE -I. -Istub -I$(PROJ)/src -I$(PROJ)/include -I$(LVGL_DIR) $(SDL_CFLAGS)
CFLAGS   := -O2 -w
CXXFLAGS := -O2 -std=c++17 -Wall -Wno-unused-function

LVGL_SRC := $(shell find $(LVGL_DIR)/src -name '*.c' 2>/dev/null)
LVGL_OBJ := $(patsubst $(LVGL_DIR)/%.c,$(BUILD)/lvgl/%.o,$(LVGL_SRC))
APP_OBJ  := $(BUILD)/ui.o $(BUILD)/model.o $(BUILD)/sim_main.o

membrane-sim: check $(LVGL_OBJ) $(APP_OBJ)
	@echo "Linking $@"
	@$(CXX) -o $@ $(APP_OBJ) $(LVGL_OBJ) $(SDL_LIBS) -lm

check:
	@test -d $(LVGL_DIR)/src || { echo "LVGL not found at $(LVGL_DIR)."; \
	  echo "Run 'pio run' in the project once, or: git clone -b v9.2.2 --depth 1 https://github.com/lvgl/lvgl.git && make LVGL_DIR=lvgl"; exit 1; }
	@command -v sdl2-config >/dev/null || { echo "SDL2 not found: brew install sdl2  (or apt install libsdl2-dev)"; exit 1; }

$(BUILD)/lvgl/%.o: $(LVGL_DIR)/%.c lv_conf.h
	@mkdir -p $(dir $@)
	@$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o: $(PROJ)/src/%.cpp $(wildcard $(PROJ)/src/*.h) $(wildcard $(PROJ)/include/*.h) lv_conf.h
	@mkdir -p $(BUILD)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILD)/sim_main.o: sim_main.cpp $(wildcard stub/*.h) lv_conf.h
	@mkdir -p $(BUILD)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

run: membrane-sim
	./membrane-sim

clean:
	rm -rf $(BUILD) membrane-sim

.PHONY: check run clean

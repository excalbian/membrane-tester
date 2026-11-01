# Desktop simulator

The simulator runs the tester's real UI and logic (`src/ui.cpp`, `src/model.cpp`) in a window on your computer, so you can try UI changes without flashing the board.

- **Mouse:** acts as the touchscreen.
- **Keyboard:** acts as the membrane switch, through a simulated PCF8574.

## Setup (once)

**macOS:**

```bash
brew install sdl2
```

**Linux:**

```bash
sudo apt install libsdl2-dev
```

**Windows:** use WSL (Ubuntu), then follow the Linux steps.

The simulator uses the LVGL copy that PlatformIO downloads for the firmware, so run `pio run` in the project once first. If you don't use PlatformIO, run `git clone -b v9.2.2 --depth 1 https://github.com/lvgl/lvgl.git` and build with `make LVGL_DIR=lvgl`.

## Run

```bash
cd sim
make run
```

- The first build takes a minute while LVGL compiles. After that, only changed files rebuild.
- The window opens at 2× scale (640 × 480).
- Close the window to quit.

## Controls

| Key                                             | Simulates                                                                             |
| ----------------------------------------------  | ------------------------------------------------------------------------------------- |
| `1 2 3 4` / `Q W E R` / `A S D F` / `Z X C V`   | A 4×4 matrix keypad: rows are pins 1–4, columns are pins 5–8                          |
| `Tab`                                           | Switch to a common-contact switch: pin 1 is common, and keys `1`–`7` connect pins 2–8 |
| `K`                                             | Toggle a short between pins 2 and 3                                                   |
| `G`                                             | Toggle pin 8 shorted to ground                                                        |
| `P`                                             | Unplug or re-plug the PCF8574                                                         |
| `H`                                             | Print the controls to the terminal                                                    |

The window title shows the switch mode, any active faults and the LED color. Beeps print to the terminal.

Profiles you save are stored in `sim/sim_nvs.bin`, so they survive restarts. Delete that file to go back to the 5 default profiles.

## Things to try

- **A passing test:** pick the *4x4 matrix* profile and press all 16 keys one at a time. You should get PASS.
- **A short:** press `K`, then start a new unit. It fails with `Short 2-3`.
- **Learn:** press `1 2 3 4 Q W E R …` in order and watch it detect the matrix.
- **Ghosting:** hold multiple keys in the same row and column to see ghosting behavior. Hold `1 2 3 4` / `Q W E R` / `A S D F` / `Z X C V` simultaneously.

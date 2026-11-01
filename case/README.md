# Case

![](case_preview.png)

The case is fully parametric (OpenSCAD). Outside size with the default values is **125.6 × 60.9 × 18.5 mm**.

- **Base:** a tray that holds the CYD on 4 pillars. It also has a bay for the PCF8574, a slot for the 8-pin DUT header on the top edge, a USB cut-out, a stylus bore along the bottom edge, and recesses for stick-on feet.
- **Bezel:** the front, with the display window (chamfered so the stylus reaches the screen edges), an engraved title, and a "1 ... 8" pin legend above the header.

The layout assumes **USB on the right (TFT_ROTATION 1)**. The PCF8574 bay and stylus entry sit at the left end. Set `usb_on_right = false` to flip it.

## Printing
| Part | File | Orientation | Notes |
|---|---|---|---|
| Base | `membrane_tester_base.stl` | floor down | no supports. The stylus bore is teardrop-shaped so it bridges cleanly |
| Bezel | `membrane_tester_bezel.stl` | face down (already oriented) | no supports. The engraved text is on the bed side |

PLA or PETG, 0.2 mm layers, 3 walls, 20 % infill.

## Hardware
- **6 × M3×10 screws** (socket or pan head), all inserted from the back:
  - 4 go through the base, through the CYD's corner holes, and into the bezel. These clamp the board.
  - 2 go into the corners of the PCF8574 bay.
  - They self-tap into the 2.6 mm pilot holes.
- **1 × 8-pin 2.54 mm header.** Use a male header if your switch tails end in a female connector, like the common black 4×4 and 3×4 keypads do. Use a female header for bare or crimped-pin tails. Push the plastic body into the slot from inside so it sits on the shelf, then fix it with a drop of CA glue. Solder wires from the short pins to P0–P7, with **pin 1 (left, marked "1") going to P0**.
- **4 × stick-on bumpers**, up to 8 mm diameter
- The PCF8574 module sits flat in the bay. Fix it with double-sided foam tape.

## Stylus
The bore is 4.5 mm for your 4 mm × 75 mm stylus. It leaves 3 mm sticking out into a finger scoop so you can pinch it. A 0.25 mm rib near the entry adds a little friction so it doesn't slide out:
- If it's too tight, set `stylus_rib = 0`.
- If it's too loose, raise `stylus_rib` to 0.35.

## Adjusting
Open `membrane_tester_case.scad` in OpenSCAD, use **Window → Customizer**, then **F6** to render and **F7** to export the STL. The values marked **MEASURE** are typical figures for the ESP32-2432S028R. Check them on your board before printing.

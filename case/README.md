# Case for Membrane Tester

![Preview render of case](case_preview.png)

The case is parametric OpenSCAD, sized from grid photos of the actual parts. Outside size is **91.6 × 93.4 × 20.7 mm**.

- **Test connector:** the membrane tail plugs straight onto the HW-171's (PCF8574)9-pin male header (J1). J1 sits in a recessed well in the front, above the screen. The mating connector drops 2.4 mm into the well and seats on J1's plastic housing, so it gets the full ~5.7 mm of pin. The tips stand 3.3 mm above the case surface.
- **Legend:** "1 … 8 · INT" is engraved under the well.
- **Module mounting:** the module keeps all its headers. It snaps into a printed **tray** with four clips, which needs no mounting holes. The I2C Dupont leads stay plugged into J3. The tray screws to the base with 2 × M3.
- **Speaker:** the 28 mm speaker sits in a ring under the tray and fires through a grille in the base.
- **Stylus:** slides into the bottom edge from the left end.
- **Orientation:** USB-C on the right (`TFT_ROTATION 1`).

## Files

| File                            | Print orientation | Notes                                             |
| ------------------------------- | ----------------- | ------------------------------------------------- |
| `membrane_tester_fit_test.stl`  | flat              | **Print first** (about 10 minutes). See below     |
| `membrane_tester_base.stl`      | floor down        | no supports                                       |
| `membrane_tester_bezel.stl`     | face down         | no supports                                       |
| `membrane_tester_tray.stl`      | floor down        | no supports. PETG gives springier clips than PLA  |

0.2 mm layers, 3 walls, 20 % infill.

## Tray

- The HW-171 rests on two edge ledges that clear the solder tails underneath. End stops locate it lengthwise.
- Press it straight down and the 4 clips snap over the board edges at spots where there are no parts.
- J5 can overhang the stop at its end, and the J3 Dupont plugs pass over the stop at the other end.
- The open centre lets wires pass through.
- Fit 2 × **M3×8** through the tray's ears into the base bosses before you plug the Dupont leads in, because the right-hand ear sits under them.
- Small stops on the underside of the bezel sit 0.2 mm above the module, so pulling a connector off can't lift it out of the clips.

## Fit test

The fit-test plate is a 1.2 mm copy of the front. To use it:

1. Lay it on the CYD and check that the 4 holes, the screen window and the USB-C notch line up.
2. Hold the HW-171 under the well (its outline is grooved into the plate) and check that the J1 pins and the jumpers sit inside the opening.

If anything is off, adjust the matching value at the top of the `.scad` file:

- Hole positions: `hole_inset_u` / `hole_inset_v`
- Screen window: `view_u` / `view_v`
- USB-C: `usb_v`
- J1 position along the module: `j1_centre_L`

## Assembly

1. Peel the green pull-tab off the screen protector. It sticks out past the top edge of the board.
2. Hot-glue the speaker into its ring, cone facing the grille, and plug it into the CYD's SPEAK connector.
3. Snap the HW-171 into the tray. Screw the tray to the base with 2 × M3×8, then plug the Dupont leads onto J3. Route the leads down past the right end of the tray to the CYD's CN1.
4. Drop the CYD onto its 4 pillars.
5. Fit the bezel, then 6 × **M3×10** from the back: 4 through the board's corner holes and 2 at the top corners.
6. Add 4 stick-on bumpers (≤ 8 mm) to lift the speaker grille off the desk.

## Check the pin legend

The pin labels were done to match my HW-171, but they may be different on your board, so check which end of J1 is P0:

1. Open **Pin monitor**.
2. Touch a wire from **GND** to the J1 pin next to the jumpers.
3. If the screen shows `GND: 1`, the default legend is right. If nothing happens (that pin is INT), set `p0_near_jumpers = false` and re-export the bezel.

## Stylus fit

The bore is 4.5 mm with a 0.25 mm friction rib:

- If it's too tight, set `stylus_rib = 0`.
- If it's too loose, set it to `0.35`.

// ============================================================================
//  Membrane Switch Tester - case
//  ESP32-2432S028R ("CYD") + PCF8574 module + 8-pin DUT header + stylus bay
//
//  Two printed parts, both print without supports:
//    base  - tray: holds the CYD on 4 pillars, PCF8574 bay, DUT header slot,
//            stylus bore along the bottom edge. Prints floor-down.
//    bezel - front: display window, title + pin legend. Prints face-down.
//
//  Assembly: 4x M3x10 through the base, through the CYD's corner holes into
//  the bezel (clamps the board), plus 2x M3x10 at the PCF8574 bay corners.
//  Screws self-tap into 2.6 mm pilot holes in PLA/PETG.
//
//  Layout (front view, USB on the right = TFT_ROTATION 1):
//
//      +--------+--------------------------------+
//      |  [hdr] |                                |
//      | PCF8574|        2.8" display            |<- USB
//      |  bay   |                                |
//      +========+================================+
//      ^ stylus goes in here, along the bottom edge
//
//  Everything marked MEASURE should be checked with calipers on your board.
// ============================================================================

/* [Render] */
part = "assembly"; // [assembly, base, bezel, print_layout]
usb_on_right = true;   // TFT_ROTATION 1 = USB on the right
explode = 14;          // assembly view only
show_board = true;     // assembly view only

/* [CYD board - MEASURE] */
pcb_l = 86.0;          // PCB length (along USB axis)
pcb_w = 50.0;          // PCB height
pcb_t = 1.6;
hole_d = 3.2;          // corner mounting holes
hole_inset_u = 3.2;    // hole centre from the PCB's short edges
hole_inset_v = 3.2;    // hole centre from the PCB's long edges
// Display module (glass + backlight), sits on the PCB's front
glass_l = 69.2;
glass_w = 50.0;
glass_t = 4.6;         // PCB front surface -> top of glass
glass_u = 8.4;         // glass edge, measured from the PCB end AWAY from USB
glass_v = 0.0;         // glass edge, from the PCB's bottom long edge
// Visible (lit) area of the screen
view_l = 57.6;
view_w = 43.2;
view_u = 14.2;         // from the PCB end AWAY from USB
view_v = 3.4;          // from the PCB bottom edge
// Rear of the board
back_clear = 8.0;      // room under the PCB for ESP32 module + JST plugs
usb_v = 25.0;          // USB connector centre, from PCB bottom edge
usb_cut_w = 12.0;      // cut-out size for the cable plug
usb_cut_h = 8.0;
usb_z = -2.0;          // cut-out centre relative to PCB rear surface (negative = behind)

/* [Case] */
wall = 2.4;
floor_t = 2.0;
front_t = 2.0;
clr = 0.4;             // gap around the PCB
glass_gap = 0.3;       // gap between glass and bezel
lap = 2.0;             // overlap joint height between base and bezel
corner_r = 4.0;
pod_l = 34.0;          // internal length of the PCF8574 bay
window_margin = 0.6;   // window oversize around the visible area

/* [Screws] */
screw_len = 10;        // M3 screw length used everywhere
m3_clear = 3.4;
m3_pilot = 2.6;
m3_head_d = 6.2;
pillar_d = 6.4;

/* [DUT header] */
hdr_pins = 8;
hdr_pitch = 2.54;
hdr_body = 2.54;       // plastic body cross-section
hdr_clr = 0.25;
hdr_z = 7.0;           // header centre height above the base's underside

/* [Stylus] */
stylus_d = 4.0;
stylus_len = 75.0;
stylus_clr = 0.5;      // bore = stylus_d + stylus_clr
stylus_protrude = 3.0; // how much sticks out to grab
stylus_rib = 0.25;     // friction rib height near the entry (0 = none)
rail_wall = 1.6;

/* [Feet / text] */
feet_d = 8.5;          // recesses for stick-on bumpers
feet_depth = 1.0;
title = "MEMBRANE TESTER";
font = "DejaVu Sans:style=Bold";

$fn = 48;
eps = 0.01;

// ---------------------------------------------------------------- derived
bore_d = stylus_d + stylus_clr;
rail = bore_d + 2 * rail_wall;            // bottom wall holds the stylus
in_y0 = rail;
in_y1 = rail + pcb_w + 2 * clr;
out_w = in_y1 + wall;
cyd_in_l = pcb_l + 2 * clr;
out_l = wall + pod_l + cyd_in_l + wall;

pod_x0 = usb_on_right ? wall : wall + cyd_in_l;     // pod inner start
cyd_x0 = usb_on_right ? wall + pod_l : wall;        // CYD bay inner start
pcb_x0 = cyd_x0 + clr;
pcb_y0 = in_y0 + clr;

z_pcb_bot = floor_t + back_clear;
z_split = z_pcb_bot + pcb_t;              // base/bezel joint = PCB front face
rim_h = glass_t + glass_gap;
z_front_in = z_split + rim_h;
out_h = z_front_in + front_t;

// screw engagement
e_pcb = min(rim_h - 0.5, screw_len - pcb_t - 3);
cb_pcb = z_pcb_bot - (screw_len - pcb_t - e_pcb);   // counterbore depth, PCB pillars
e_pod = min(rim_h - 0.5, screw_len - 3);
cb_pod = z_split - (screw_len - e_pod);
pilot_depth = rim_h + min(0.8, front_t - 1.2);

echo(str("Case outside: ", out_l, " x ", out_w, " x ", out_h, " mm"));
echo(str("Counterbores: PCB ", cb_pcb, " mm, pod ", cb_pod, " mm"));
assert(cb_pcb > 0.5 && cb_pod > 0.5, "screw_len too long for this stack");

// board-local (u from the end away from USB, v from bottom edge) -> case XY
function bx(u, l = 0) = usb_on_right ? pcb_x0 + u : pcb_x0 + pcb_l - u - l;
function by(v, w = 0) = usb_on_right ? pcb_y0 + v : pcb_y0 + pcb_w - v - w;

pcb_holes = [for (u = [hole_inset_u, pcb_l - hole_inset_u])
             for (v = [hole_inset_v, pcb_w - hole_inset_v]) [bx(u), by(v)]];
pod_px = usb_on_right ? wall + pillar_d / 2 - 0.6 : out_l - wall - pillar_d / 2 + 0.6;   // sunk into the wall
pod_holes = [[pod_px, in_y0 + pillar_d / 2 - 0.6], [pod_px, in_y1 - pillar_d / 2 + 0.6]];
pod_cx = pod_x0 + pod_l / 2;
hdr_len = hdr_pins * hdr_pitch;

// ---------------------------------------------------------------- helpers
module rbox(l, w, h, r) {
  hull() for (x = [r, l - r], y = [r, w - r]) translate([x, y, 0]) cylinder(r = r, h = h);
}
module rbox_inset(inset, h) {
  translate([inset, inset, 0]) rbox(out_l - 2 * inset, out_w - 2 * inset, h, max(0.5, corner_r - inset));
}
module teardrop2d(r) { hull() { circle(r = r); translate([-r * sqrt(2), 0]) square(eps, center = true); } }

// ---------------------------------------------------------------- base
module base() {
  bore_z = z_split / 2 + 0.4;
  bore_y = rail / 2;
  bore_depth = stylus_len - stylus_protrude + 0.5;
  entry_x = usb_on_right ? 0 : out_l;
  dir = usb_on_right ? 1 : -1;

  difference() {
    union() {
      difference() {
        rbox(out_l, out_w, z_split, corner_r);
        translate([wall, in_y0, floor_t]) cube([out_l - 2 * wall, in_y1 - in_y0, z_split]);
      }
      // outer skin of the lap joint
      translate([0, 0, z_split - eps]) difference() {
        rbox_inset(0, lap);
        translate([0, 0, -1]) rbox_inset(wall / 2 - 0.1, lap + 2);
      }
      // CYD pillars (top = PCB rear face)
      for (p = pcb_holes) translate([p[0], p[1], 0]) cylinder(d = pillar_d, h = z_pcb_bot);
      // pod pillars (full height)
      for (p = pod_holes) translate([p[0], p[1], 0]) cylinder(d = pillar_d, h = z_split);
      // shelf under the DUT header
      translate([pod_cx - hdr_len / 2 - 2, in_y1 - 3, 0])
        cube([hdr_len + 4, 3 + eps, hdr_z - hdr_body / 2 - hdr_clr]);
    }
    // screw holes + counterbores from below
    for (p = pcb_holes) translate([p[0], p[1], -1]) {
      cylinder(d = m3_clear, h = z_split + 2);
      cylinder(d = m3_head_d, h = cb_pcb + 1);
    }
    for (p = pod_holes) translate([p[0], p[1], -1]) {
      cylinder(d = m3_clear, h = z_split + 2);
      cylinder(d = m3_head_d, h = cb_pod + 1);
    }
    // DUT header slot in the top wall
    translate([pod_cx - hdr_len / 2 - hdr_clr, in_y1 - 3.5, hdr_z - hdr_body / 2 - hdr_clr])
      cube([hdr_len + 2 * hdr_clr, wall + 3.5 + 1, hdr_body + 2 * hdr_clr]);
    // USB cut-out
    usb_x = usb_on_right ? out_l - wall - 1 : -1;
    translate([usb_x, by(usb_v) - usb_cut_w / 2, z_pcb_bot + usb_z - usb_cut_h / 2])
      cube([wall + 2 + clr, usb_cut_w, usb_cut_h]);
    // stylus bore (teardrop so it prints without support) + lead-in chamfer
    translate([entry_x - dir * eps, bore_y, bore_z]) rotate([0, dir * 90, 0]) {
      rotate([0, 0, usb_on_right ? 0 : 180]) linear_extrude(bore_depth) teardrop2d(bore_d / 2);
      cylinder(d1 = bore_d + 1.6, d2 = bore_d, h = 0.8);
    }
    // finger scoop so the stylus end can be pinched
    translate([entry_x, bore_y, bore_z]) scale([1, 1, 1.3]) sphere(d = 10);
    // feet recesses
    for (x = [out_l * 0.16, out_l * 0.8], y = [12, out_w - 12]) translate([x, y, -eps]) cylinder(d = feet_d, h = feet_depth);
  }
  // friction rib inside the bore, just past the entry
  if (stylus_rib > 0)
    translate([usb_on_right ? 6 : out_l - 12, bore_y - 0.4, bore_z - bore_d / 2 - eps])
      cube([6, 0.8, stylus_rib + eps]);
}

// ---------------------------------------------------------------- bezel
module bezel() {
  difference() {
    union() {
      difference() {
        translate([0, 0, z_split]) rbox(out_l, out_w, out_h - z_split, corner_r);
        // pocket for board + glass + pod
        translate([wall, in_y0, z_split - 1]) cube([out_l - 2 * wall, in_y1 - in_y0, rim_h + 1]);
        // rebate for the lap joint
        translate([0, 0, z_split - 1]) difference() {
          translate([-1, -1, 0]) cube([out_l + 2, out_w + 2, lap + 1.1]);
          translate([0, 0, -1]) rbox_inset(wall / 2 + 0.1, lap + 4);
        }
      }
      for (p = concat(pcb_holes, pod_holes))
        translate([p[0], p[1], z_split]) cylinder(d = pillar_d, h = rim_h + eps);
    }
    // pilot holes
    for (p = concat(pcb_holes, pod_holes))
      translate([p[0], p[1], z_split - eps]) cylinder(d = m3_pilot, h = pilot_depth);
    // display window, 45 deg chamfer outward for stylus reach at the edges
    wx = bx(view_u, view_l) - window_margin;
    wy = by(view_v, view_w) - window_margin;
    wl = view_l + 2 * window_margin;
    ww = view_w + 2 * window_margin;
    hull() {
      translate([wx, wy, z_front_in - 1]) cube([wl, ww, 1 + eps]);
      translate([wx - front_t, wy - front_t, out_h - eps]) cube([wl + 2 * front_t, ww + 2 * front_t, eps * 2]);
    }
    // engraved title (vertical, on the pod) + pin legend over the header
    translate([pod_cx + 2.0, in_y0 + 22, out_h - 0.6])
      linear_extrude(1) rotate(90) text(title, size = 3.6, font = font, halign = "center", valign = "center");
    for (i = [0, hdr_pins - 1])
      translate([pod_cx - hdr_len / 2 + hdr_pitch * (i + 0.5), in_y1 - 3.2, out_h - 0.6])
        linear_extrude(1) text(str(i + 1), size = 3.2, font = font, halign = "center", valign = "center");
    translate([pod_cx - hdr_len / 2 + hdr_pitch * 1.5, in_y1 - 3.45, out_h - 0.6])
      cube([hdr_pitch * (hdr_pins - 3), 0.5, 1]);
  }
}

// ---------------------------------------------------------------- mock-ups
module board() {
  color("seagreen") difference() {
    translate([pcb_x0, pcb_y0, z_pcb_bot]) cube([pcb_l, pcb_w, pcb_t]);
    for (p = pcb_holes) translate([p[0], p[1], z_pcb_bot - 1]) cylinder(d = hole_d, h = 4);
  }
  color("dimgray") translate([bx(glass_u, glass_l), by(glass_v, glass_w), z_split]) cube([glass_l, glass_w, glass_t]);
  color("steelblue") translate([bx(view_u, view_l), by(view_v, view_w), z_split + glass_t - 0.1]) cube([view_l, view_w, 0.15]);
  color("silver") translate([usb_on_right ? pcb_x0 + pcb_l - 5 : pcb_x0 - 1, by(usb_v) - 4, z_pcb_bot - 3]) cube([6, 8, 3]);
  color("orange") translate([bx(30, 18), by(16, 25.5), z_pcb_bot - 3.2]) cube([18, 25.5, 3.2]);   // ESP32 module (approx)
  color("royalblue") translate([pod_cx - 13, in_y0 + 6, floor_t + 1]) cube([26, 38, 1.6]);         // PCF8574 module (approx)
  color("black") translate([pod_cx - hdr_len / 2, in_y1 - 0.5, hdr_z - hdr_body / 2]) cube([hdr_len, hdr_body, hdr_body]);
  color("gold") for (i = [0 : hdr_pins - 1])
    translate([pod_cx - hdr_len / 2 + hdr_pitch * (i + 0.5) - 0.32, in_y1 - 3, hdr_z - 0.32]) cube([0.64, 11.5, 0.64]);
  color("white") translate([usb_on_right ? -stylus_protrude : out_l - stylus_len + stylus_protrude, rail / 2, z_split / 2 + 0.4])
    rotate([0, 90, 0]) cylinder(d = stylus_d, h = stylus_len);
}

// ---------------------------------------------------------------- output
if (part == "base") base();
else if (part == "bezel") translate([0, out_w, out_h]) rotate([180, 0, 0]) bezel();   // face-down for printing
else if (part == "print_layout") {
  base();
  translate([0, out_w * 2 + 10, out_h]) rotate([180, 0, 0]) bezel();
} else {
  color("#3a3f47") base();
  if (show_board) board();
  color("#555c66", 0.9) translate([0, 0, explode]) bezel();
}

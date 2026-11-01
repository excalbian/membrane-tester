// ============================================================================
//  Membrane Switch Tester - case  (v2, dimensions from Adam's grid photos)
//  ESP32-2432S028R ("CYD", USB-C) + HW-171 PCF8574 module + 28 mm speaker
//
//  The membrane tail plugs straight onto the PCF8574's own 9-pin male header
//  (J1), which pokes out through the front of the case beside the screen.
//
//      +-----------+---------------------------------+
//      | 1 o  [jp] |                                 |
//      | 2 o  chip |                                 |
//      | 3 o       |        2.8" display             |=< USB-C
//      | ...       |                                 |
//      | INT o     |                                 |
//      +===========+=================================+
//      ^ stylus slides in here (bottom edge)    speaker grille underneath
//
//  Parts (both print without supports):
//    base     - floor-down. CYD pillars, speaker ring + grille, module posts,
//               stylus bore, USB-C cut-out.
//    bezel    - face-down. Screen window, J1 window, legend, title.
//    fit_test - 1.2 mm plate of the front: print it first and lay it on the
//               board to check holes / window / J1 before the full print.
//
//  Module prep: snip off J3 (right-angle I2C pins) and J5 (female I2C
//  header) flush with the board and solder the 4 I2C wires to the J3 holes
//  from underneath. Nothing else on the module changes.
//
//  Screws: 6x M3x10, all from the back, self-tapping into the bezel.
// ============================================================================

/* [Render] */
part = "assembly"; // [assembly, base, bezel, print_layout, fit_test]
usb_on_right = true;     // TFT_ROTATION 1
explode = 16;
show_parts = true;

/* [CYD board] */
pcb_l = 86.0;
pcb_w = 50.0;
pcb_t = 1.6;
hole_d = 3.2;
hole_inset_u = 3.5;      // hole centre from the short edges
hole_inset_v = 3.7;      // hole centre from the long edges
glass_l = 70.1;
glass_w = 50.0;
glass_t = 4.8;           // PCB front -> top of touch glass
glass_u = 8.4;           // from the PCB end away from USB
glass_v = 0.0;
view_l = 57.6;
view_w = 43.2;
view_u = 11.4;           // lit area, from the end away from USB
view_v = 3.5;
back_clear = 10.0;       // tallest rear part incl. plugged JST leads
usb_v = 22.8;            // USB-C centre from the PCB bottom edge
usb_cut_w = 13.0;
usb_cut_h = 8.5;
usb_z = -1.8;            // centre relative to the PCB rear face

/* [PCF8574 module (HW-171)] */
mod_l = 48.0;
mod_w = 15.6;
mod_t = 1.6;
mod_tails = 3.5;         // solder tails under the board
mod_gap = 6.4;           // gap between the pod's outer wall and the module
p0_at_top = true;        // true: P0 is the J1 pin next to the jumpers / "J1" label
j1_pitch = 2.54;
j1_centre_L = 28.0;      // centre of the 9-pin row along the module
j1_w = 1.95;             // pin row distance from the module's long edge

/* [Speaker] */
spk_d = 28.0;
spk_t = 5.0;
grille_r = 11.0;

/* [Case] */
wall = 2.4;
floor_t = 2.0;
front_t = 2.0;
clr = 0.4;
glass_gap = 0.3;
lap = 2.0;
corner_r = 4.0;
pod_l = 30.0;
window_margin = 0.6;

/* [Screws] */
screw_len = 10;
m3_clear = 3.6;          // a little generous for hole-position tolerance
m3_pilot = 2.6;
m3_head_d = 6.2;
pillar_d = 6.4;

/* [Stylus] */
stylus_d = 4.0;
stylus_len = 75.0;
stylus_clr = 0.5;
stylus_protrude = 3.0;
stylus_rib = 0.25;
rail_wall = 1.6;

/* [Feet / text] */
feet_d = 8.5;
feet_depth = 1.0;
title = "MEMBRANE TESTER";
font = "DejaVu Sans:style=Bold";

$fn = 48;
eps = 0.01;

// ---------------------------------------------------------------- derived
bore_d = stylus_d + stylus_clr;
rail = bore_d + 2 * rail_wall;
in_y0 = rail;
in_y1 = rail + pcb_w + 2 * clr;
out_w = in_y1 + wall;
cyd_in_l = pcb_l + 2 * clr;
out_l = wall + pod_l + cyd_in_l + wall;

pod_x0 = usb_on_right ? wall : wall + cyd_in_l;
cyd_x0 = usb_on_right ? wall + pod_l : wall;
pcb_x0 = cyd_x0 + clr;
pcb_y0 = in_y0 + clr;

z_pcb_bot = floor_t + back_clear;
z_split = z_pcb_bot + pcb_t;
rim_h = glass_t + glass_gap;
z_front_in = z_split + rim_h;
out_h = z_front_in + front_t;
z_mod_bot = z_front_in - mod_t;          // module top touches the front plate

e_pcb = min(rim_h - 0.5, screw_len - pcb_t - 3);
cb_pcb = z_pcb_bot - (screw_len - pcb_t - e_pcb);
e_pod = min(rim_h - 0.5, screw_len - 3);
cb_pod = z_split - (screw_len - e_pod);
pilot_depth = rim_h + min(0.8, front_t - 1.2);

echo(str("Case outside: ", out_l, " x ", out_w, " x ", out_h, " mm"));
echo(str("J1 pins stand ", 8.3 - front_t, " mm proud of the front"));
assert(cb_pcb > 0.5 && cb_pod > 0.5, "screw_len too long for this stack");
assert(z_mod_bot - mod_tails > floor_t + spk_t + 1, "no room for the speaker under the module");

function bx(u, l = 0) = usb_on_right ? pcb_x0 + u : pcb_x0 + pcb_l - u - l;
function by(v, w = 0) = usb_on_right ? pcb_y0 + v : pcb_y0 + pcb_w - v - w;

pcb_holes = [for (u = [hole_inset_u, pcb_l - hole_inset_u])
             for (v = [hole_inset_v, pcb_w - hole_inset_v]) [bx(u), by(v)]];
pod_px = usb_on_right ? wall + pillar_d / 2 - 0.6 : out_l - wall - pillar_d / 2 + 0.6;
pod_holes = [[pod_px, in_y0 + pillar_d / 2 - 0.6], [pod_px, in_y1 - pillar_d / 2 + 0.6]];
pod_cx = pod_x0 + pod_l / 2;
mid_y = (in_y0 + in_y1) / 2;

// Module frame: local x = L (along the module, J5 end = 0), local y = W
// (across, J1 edge = 0). J1 always faces the pod's outer wall.
my_top = mid_y + mod_l / 2;
my_bot = mid_y - mod_l / 2;
mx_out = usb_on_right ? wall + mod_gap : out_l - wall - mod_gap;
function mpos(L, W) = usb_on_right ? [mx_out + W, my_top - L] : [mx_out - W, my_bot + L];
module in_mod(z = 0) {
  if (usb_on_right) translate([mx_out, my_top, z]) rotate(-90) children();
  else translate([mx_out, my_bot, z]) rotate(90) children();
}
// local rectangle [L0, W0, L1, W1]
module mrect(r, h) translate([r[0], r[1], 0]) cube([r[2] - r[0], r[3] - r[1], h]);

mod_window  = [13.3, -0.3, 44.0, mod_w + 0.3];     // J1 + jumpers + chip + R1/R2
mod_pockets = [[-0.3, 2.3, 3.6, 13.3],               // snipped J5 stubs
               [44.0, 3.3, mod_l + 0.3, 14.3]];      // snipped J3 stubs + wires
mod_posts = [[6, 1.8], [6, mod_w - 1.8], [42.5, 1.8], [42.5, mod_w - 1.8]];
function j1_L(i) = j1_centre_L + (i - 4) * j1_pitch;
function j1_label(i) = p0_at_top ? (i < 8 ? str(i + 1) : "INT")
                                 : (i == 0 ? "INT" : str(9 - i));

// ---------------------------------------------------------------- helpers
module rbox(l, w, h, r) {
  hull() for (x = [r, l - r], y = [r, w - r]) translate([x, y, 0]) cylinder(r = r, h = h);
}
module rbox_inset(inset, h) {
  translate([inset, inset, 0]) rbox(out_l - 2 * inset, out_w - 2 * inset, h, max(0.5, corner_r - inset));
}
module teardrop2d(r) { hull() { circle(r = r); translate([-r * sqrt(2), 0]) square(eps, center = true); } }

module usb_cut(z0, z1) {
  ux = usb_on_right ? out_l - wall - clr - 1 : -1;
  translate([ux, by(usb_v) - usb_cut_w / 2, z0]) cube([wall + clr + 2, usb_cut_w, z1 - z0]);
}

// ---------------------------------------------------------------- base
module base() {
  bore_z = z_split / 2;
  bore_y = rail / 2;
  bore_depth = stylus_len - stylus_protrude + 0.5;
  entry_x = usb_on_right ? 0 : out_l;
  dir = usb_on_right ? 1 : -1;
  spk_ring_h = 3.0;

  difference() {
    union() {
      difference() {
        rbox(out_l, out_w, z_split, corner_r);
        translate([wall, in_y0, floor_t]) cube([out_l - 2 * wall, in_y1 - in_y0, z_split]);
      }
      translate([0, 0, z_split - eps]) difference() {
        rbox_inset(0, lap);
        translate([0, 0, -1]) rbox_inset(wall / 2 - 0.1, lap + 2);
      }
      for (p = pcb_holes) translate([p[0], p[1], 0]) cylinder(d = pillar_d, h = z_pcb_bot);
      for (p = pod_holes) translate([p[0], p[1], 0]) cylinder(d = pillar_d, h = z_split);
      // speaker retaining ring
      translate([pod_cx, mid_y, 0]) difference() {
        cylinder(d = spk_d + 0.6 + 2.4, h = floor_t + spk_ring_h);
        translate([0, 0, floor_t]) cylinder(d = spk_d + 0.6, h = spk_ring_h + 1);
      }
      // posts that push the module up against the front plate
      for (q = mod_posts) {
        p = mpos(q[0], q[1]);
        hull() {
          translate([p[0] - 2.5, p[1] - 2.5, 0]) cube([5, 5, floor_t + 1]);
          translate([p[0] - 1.5, p[1] - 1.5, 0]) cube([3, 3, z_mod_bot]);
        }
      }
    }
    for (p = pcb_holes) translate([p[0], p[1], -1]) {
      cylinder(d = m3_clear, h = z_split + 2);
      cylinder(d = m3_head_d, h = cb_pcb + 1);
    }
    for (p = pod_holes) translate([p[0], p[1], -1]) {
      cylinder(d = m3_clear, h = z_split + 2);
      cylinder(d = m3_head_d, h = cb_pod + 1);
    }
    usb_cut(z_pcb_bot + usb_z - usb_cut_h / 2, z_split + lap + 1);
    // speaker grille
    for (i = [-4 : 4], j = [-4 : 4]) {
      x = (i + (j % 2) * 0.5) * 3.2;
      y = j * 3.2 * sin(60);
      if (sqrt(x * x + y * y) <= grille_r) translate([pod_cx + x, mid_y + y, -1]) cylinder(d = 2.0, h = floor_t + 2, $fn = 16);
    }
    // stylus bore + lead-in + finger scoop
    translate([entry_x - dir * eps, bore_y, bore_z]) rotate([0, dir * 90, 0]) {
      rotate([0, 0, usb_on_right ? 0 : 180]) linear_extrude(bore_depth) teardrop2d(bore_d / 2);
      cylinder(d1 = bore_d + 1.6, d2 = bore_d, h = 0.8);
    }
    translate([entry_x, bore_y, bore_z]) scale([1, 1, 1.3]) sphere(d = 10);
    // feet
    for (x = [out_l * 0.3, out_l * 0.85], y = [12, out_w - 12]) translate([x, y, -eps]) cylinder(d = feet_d, h = feet_depth);
  }
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
        translate([wall, in_y0, z_split - 1]) cube([out_l - 2 * wall, in_y1 - in_y0, rim_h + 1]);
        translate([0, 0, z_split - 1]) difference() {
          translate([-1, -1, 0]) cube([out_l + 2, out_w + 2, lap + 1.1]);
          translate([0, 0, -1]) rbox_inset(wall / 2 + 0.1, lap + 4);
        }
      }
      for (p = concat(pcb_holes, pod_holes))
        translate([p[0], p[1], z_split]) cylinder(d = pillar_d, h = rim_h + eps);
      // locating frame around the module (hangs 1.5 mm below the front plate)
      in_mod(z_front_in - 1.5) difference() {
        translate([-1.5, -1.5, 0]) cube([mod_l + 3, mod_w + 3, 1.5 + eps]);
        translate([-0.3, -0.3, -1]) cube([mod_l + 0.6, mod_w + 0.6, 4]);
      }
    }
    for (p = concat(pcb_holes, pod_holes))
      translate([p[0], p[1], z_split - eps]) cylinder(d = m3_pilot, h = pilot_depth);
    usb_cut(z_split - 2, z_pcb_bot + usb_z + usb_cut_h / 2);
    front_openings(out_h - z_front_in + 2);
    // pockets over the snipped header stubs
    in_mod(z_front_in - 1) for (r = mod_pockets) mrect(r, 1.8);
    // title (vertical, between module and screen)
    tx = usb_on_right ? pod_x0 + pod_l - 2.6 : pod_x0 + 2.6;
    translate([tx, mid_y, out_h - 0.6]) linear_extrude(1)
      rotate(usb_on_right ? 90 : -90) text(title, size = 3.4, font = font, halign = "center", valign = "center");
    // J1 pin legend
    for (i = [0 : 8]) {
      p = mpos(j1_L(i), -3.4);
      lbl = j1_label(i);
      translate([p[0], p[1], out_h - 0.6]) linear_extrude(1)
        text(lbl, size = lbl == "INT" ? 1.5 : 2.0, font = font, halign = "center", valign = "center");
    }
  }
}

module front_openings(h) {
  wx = bx(view_u, view_l) - window_margin;
  wy = by(view_v, view_w) - window_margin;
  wl = view_l + 2 * window_margin;
  ww = view_w + 2 * window_margin;
  hull() {
    translate([wx, wy, z_front_in - 1]) cube([wl, ww, 1 + eps]);
    translate([wx - front_t, wy - front_t, out_h - eps]) cube([wl + 2 * front_t, ww + 2 * front_t, eps * 2]);
  }
  in_mod(z_front_in - 1) mrect(mod_window, h + 1);
}

// 1.2 mm template of the front face: holes, windows and outlines to check
module fit_test() {
  t = 1.2;
  difference() {
    rbox(out_l, out_w, t, corner_r);
    translate([0, 0, -(z_front_in - 1) - 0.5]) front_openings(10);
    for (p = concat(pcb_holes, pod_holes)) translate([p[0], p[1], -1]) cylinder(d = hole_d, h = t + 2);
    // outlines of the PCB, glass and module (0.4 mm grooves)
    translate([0, 0, t - 0.4]) linear_extrude(1) {
      difference() { translate([pcb_x0, pcb_y0]) square([pcb_l, pcb_w]); translate([pcb_x0 + 0.5, pcb_y0 + 0.5]) square([pcb_l - 1, pcb_w - 1]); }
      difference() { translate([bx(glass_u, glass_l), by(glass_v, glass_w)]) square([glass_l, glass_w]);
                     translate([bx(glass_u, glass_l) + 0.5, by(glass_v, glass_w) + 0.5]) square([glass_l - 1, glass_w - 1]); }
    }
    // USB-C position notch on the edge
    translate([usb_on_right ? out_l - 3 : -1, by(usb_v) - usb_cut_w / 2, -1]) cube([4, usb_cut_w, t + 2]);
  }
}

// ---------------------------------------------------------------- mock-ups
module mockups() {
  color("seagreen") difference() {
    translate([pcb_x0, pcb_y0, z_pcb_bot]) cube([pcb_l, pcb_w, pcb_t]);
    for (p = pcb_holes) translate([p[0], p[1], z_pcb_bot - 1]) cylinder(d = hole_d, h = 4);
  }
  color("dimgray") translate([bx(glass_u, glass_l), by(glass_v, glass_w), z_split]) cube([glass_l, glass_w, glass_t]);
  color("steelblue") translate([bx(view_u, view_l), by(view_v, view_w), z_split + glass_t - 0.1]) cube([view_l, view_w, 0.15]);
  color("silver") translate([usb_on_right ? pcb_x0 + pcb_l - 7 : pcb_x0 - 1, by(usb_v) - 4.5, z_pcb_bot + usb_z - 1.6]) cube([8, 9, 3.2]);
  // HW-171
  in_mod(z_mod_bot) {
    color("navy") cube([mod_l, mod_w, mod_t]);
    color("yellow") translate([j1_L(0) - 1.27, j1_w - 1.27, mod_t]) cube([9 * j1_pitch, 2.54, 2.6]);
    color("gold") for (i = [0 : 8]) translate([j1_L(i) - 0.32, j1_w - 0.32, mod_t]) cube([0.64, 0.64, 8.3]);
    color("yellow") translate([14, 5.8, mod_t]) cube([4, 8.5, 7.0]);
    color("black") translate([22, 5.5, mod_t]) cube([9.5, 9, 1.7]);
  }
  color("black") translate([pod_cx, mid_y, floor_t]) cylinder(d = spk_d, h = spk_t);
  color("white") translate([usb_on_right ? -stylus_protrude : out_l - stylus_len + stylus_protrude, rail / 2, z_split / 2])
    rotate([0, 90, 0]) cylinder(d = stylus_d, h = stylus_len);
}

// ---------------------------------------------------------------- output
if (part == "base") base();
else if (part == "bezel") translate([0, out_w, out_h]) rotate([180, 0, 0]) bezel();
else if (part == "fit_test") translate([0, out_w, 1.2]) rotate([180, 0, 0]) fit_test();
else if (part == "print_layout") {
  base();
  translate([0, out_w * 2 + 10, out_h]) rotate([180, 0, 0]) bezel();
} else {
  color("#3a3f47") base();
  if (show_parts) mockups();
  color("#555c66", 0.85) translate([0, 0, explode]) bezel();
}

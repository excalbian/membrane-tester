// ============================================================================
//  Membrane Switch Tester - case  (v3)
//  ESP32-2432S028R ("CYD", USB-C) + HW-171 PCF8574 module + 28 mm speaker
//
//  The membrane tail plugs straight onto the module's 9-pin male header (J1)
//  through a recessed well in the front, above the screen. The module keeps
//  all its headers: it snaps into a printed tray (4 clips, no holes needed),
//  and the I2C Dupont leads stay plugged into J3.
//
//      +-----------------------------------------+
//      |  [ J1 well: o o o o o o o o o ]  MEMBRANE|
//      |    1 2 3 4 5 6 7 8 INT            TESTER |
//      | +-------------------------------------+ |
//      | |                                     | |
//      | |          2.8" display               | |=< USB-C
//      | |                                     | |
//      | +-------------------------------------+ |
//      +=========================================+
//      ^ stylus slides in here (bottom edge)
//
//  Parts (all print without supports):
//    base     - floor-down: CYD pillars, speaker ring + grille, tray bosses,
//               stylus bore, USB-C cut-out
//    bezel    - face-down: screen window, J1 well, legend, hold-down stops
//    tray     - floor-down: snap-in cradle for the HW-171, 2x M3 to the base
//    fit_test - 1.2 mm template of the front, print first
//
//  Screws: 6x M3x10 (case, from the back) + 2x M3x8 (tray, from the front)
// ============================================================================

/* [Render] */
part = "assembly"; // [assembly, base, bezel, tray, print_layout, fit_test]
usb_on_right = true;     // TFT_ROTATION 1
explode = 16;
show_parts = true;

/* [CYD board] */
pcb_l = 86.0;
pcb_w = 50.0;
pcb_t = 1.6;
hole_d = 3.2;
hole_inset_u = 3.5;
hole_inset_v = 3.7;
glass_l = 70.1;
glass_w = 50.0;
glass_t = 4.8;
glass_u = 8.4;           // from the PCB end away from USB
glass_v = 0.0;
view_l = 57.6;
view_w = 43.2;
view_u = 11.4;
view_v = 3.5;
back_clear = 10.0;
usb_v = 22.8;
usb_cut_w = 13.0;
usb_cut_h = 8.5;
usb_z = -1.8;

/* [PCF8574 module (HW-171)] */
mod_l = 48.0;            // PCB length (J5 end = 0, J3 end = 48)
mod_w = 15.6;            // PCB width (J1 edge = 0)
mod_t = 1.6;
mod_tails = 3.5;         // solder tails under the board
j5_overhang = 0.8;       // J5 socket sticks out past the PCB end
j3_reach = 16.0;         // J3 pins + Dupont housings beyond the PCB end
wire_bend = 5.0;         // room for the Dupont leads to turn down
plug_h = 2.6;            // Dupont housing height above the PCB
j3_housing_h = 2.3;
p0_near_jumpers = true;  // true: P0 is the J1 pin next to the jumpers / "J1" label
j1_pitch = 2.54;
j1_centre_L = 28.0;
j1_w = 1.95;
mod_x0 = 14.0;           // case X of the module's J5 end

/* [Tray] */
tray_floor = 1.6;
ledge_h = 4.0;           // clears the solder tails
clip_l = 5.0;
clip_t = 1.2;
hook = 0.7;              // how far the clip hooks over the PCB edge

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
strip_in = 32.5;         // inner height of the module strip above the screen
window_margin = 0.6;
well_margin = 3.0;       // room around J1 for the mating connector
mod_top_gap = 3.0;       // module top -> underside of front plate (plug clearance)

/* [Screws] */
screw_len = 10;
m3_clear = 3.6;
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
font = "DejaVu Sans:style=Bold";

$fn = 48;
eps = 0.01;

// ---------------------------------------------------------------- derived
bore_d = stylus_d + stylus_clr;
rail = bore_d + 2 * rail_wall;
in_y0 = rail;                            // CYD bay bottom
in_y1 = rail + pcb_w + 2 * clr;          // CYD bay top / strip bottom
in_y2 = in_y1 + strip_in;                // strip top
out_w = in_y2 + wall;
cyd_in_l = pcb_l + 2 * clr;
out_l = wall + cyd_in_l + wall;

pcb_x0 = wall + clr;
pcb_y0 = in_y0 + clr;

z_pcb_bot = floor_t + back_clear;
z_split = z_pcb_bot + pcb_t;
rim_h = glass_t + glass_gap;
z_front_in = z_split + rim_h;
out_h = z_front_in + front_t;

z_mod_top = z_front_in - mod_top_gap;
z_mod_bot = z_mod_top - mod_t;
z_tray = z_mod_bot - ledge_h - tray_floor;   // tray underside

e_pcb = min(rim_h - 0.5, screw_len - pcb_t - 3);
cb_pcb = z_pcb_bot - (screw_len - pcb_t - e_pcb);
e_cs = min(rim_h - 0.5, screw_len - 3);
cb_cs = z_split - (screw_len - e_cs);
pilot_depth = rim_h + min(0.8, front_t - 1.2);

function bx(u, l = 0) = usb_on_right ? pcb_x0 + u : pcb_x0 + pcb_l - u - l;
function by(v, w = 0) = usb_on_right ? pcb_y0 + v : pcb_y0 + pcb_w - v - w;

// module frame: local x = L along the module, local y = W across (J1 edge = 0)
view_top = by(view_v, view_w) + view_w + window_margin + front_t;   // screen chamfer top
mod_y0 = view_top + 1.0 + 2.2 + 1.8 + well_margin;                    // J1 edge, leaves room for the legend
function mpos(L, W) = [mod_x0 + L, mod_y0 + W];
module in_mod(z = 0) translate([mod_x0, mod_y0, z]) children();
module mrect(r, h) translate([r[0], r[1], 0]) cube([r[2] - r[0], r[3] - r[1], h]);

well = [13.3, -well_margin, 44.0, mod_w + 0.3];
tray_ears = [[-5.0, mod_w / 2], [53.0, mod_w + 1.6]];    // L, W of the tray screw holes
holddowns = [[9, 3.0], [9, 12.5]];                        // bezel stops onto bare PCB
j3_pad = [45.0, 5.0, 47.5, 13.0];                          // bezel stop onto J3's housing
function j1_L(i) = j1_centre_L + (i - 4) * j1_pitch;
function j1_label(i) = p0_near_jumpers ? (i < 8 ? str(i + 1) : "INT")
                                       : (i == 0 ? "INT" : str(9 - i));

pcb_holes = [for (u = [hole_inset_u, pcb_l - hole_inset_u])
             for (v = [hole_inset_v, pcb_w - hole_inset_v]) [bx(u), by(v)]];
cs_holes = [[wall + pillar_d / 2 - 0.6, in_y2 - pillar_d / 2 + 0.6],
            [out_l - wall - pillar_d / 2 + 0.6, in_y2 - pillar_d / 2 + 0.6]];
spk_c = [mod_x0 + mod_l / 2, in_y2 - spk_d / 2 - 1.8];

echo(str("Case outside: ", out_l, " x ", out_w, " x ", out_h, " mm"));
echo(str("J1 pin tips ", z_mod_top + 8.3 - out_h, " mm above the front; well depth to J1 housing ", out_h - (z_mod_top + 2.6), " mm"));
assert(cb_pcb > 0.5 && cb_cs > 0.5, "screw_len too long");
assert(mod_x0 + mod_l + j3_reach + wire_bend <= out_l - wall, "J3 leads hit the right wall: reduce mod_x0");
assert(mod_x0 - 8 >= wall, "tray ear hits the left wall: increase mod_x0");
assert(mod_y0 + mod_w + 2 <= in_y2, "module strip too short");
assert(z_tray > floor_t + spk_t + 0.5, "tray sits on the speaker");
assert(z_mod_top + plug_h < z_front_in, "Dupont plugs hit the front plate");

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
module cavity(z0, h) translate([wall, in_y0, z0]) cube([out_l - 2 * wall, in_y2 - in_y0, h]);

// ---------------------------------------------------------------- base
module base() {
  bore_z = z_split / 2;
  bore_y = rail / 2;
  bore_depth = stylus_len - stylus_protrude + 0.5;
  entry_x = usb_on_right ? 0 : out_l;
  dir = usb_on_right ? 1 : -1;

  difference() {
    union() {
      difference() {
        rbox(out_l, out_w, z_split, corner_r);
        cavity(floor_t, z_split);
      }
      translate([0, 0, z_split - eps]) difference() {
        rbox_inset(0, lap);
        translate([0, 0, -1]) rbox_inset(wall / 2 - 0.1, lap + 2);
      }
      for (p = pcb_holes) translate([p[0], p[1], 0]) cylinder(d = pillar_d, h = z_pcb_bot);
      for (p = cs_holes) translate([p[0], p[1], 0]) cylinder(d = pillar_d, h = z_split);
      // speaker ring
      translate([spk_c[0], spk_c[1], 0]) difference() {
        cylinder(d = spk_d + 0.6 + 2.4, h = floor_t + 3);
        translate([0, 0, floor_t]) cylinder(d = spk_d + 0.6, h = 4);
      }
      // tray bosses
      for (e = tray_ears) { p = mpos(e[0], e[1]); translate([p[0], p[1], 0]) cylinder(d = 7, h = z_tray); }
    }
    for (p = pcb_holes) translate([p[0], p[1], -1]) { cylinder(d = m3_clear, h = z_split + 2); cylinder(d = m3_head_d, h = cb_pcb + 1); }
    for (p = cs_holes) translate([p[0], p[1], -1]) { cylinder(d = m3_clear, h = z_split + 2); cylinder(d = m3_head_d, h = cb_cs + 1); }
    for (e = tray_ears) { p = mpos(e[0], e[1]); translate([p[0], p[1], z_tray - 7]) cylinder(d = m3_pilot, h = 8); }
    usb_cut(z_pcb_bot + usb_z - usb_cut_h / 2, z_split + lap + 1);
    for (i = [-4 : 4], j = [-4 : 4]) {
      x = (i + (j % 2) * 0.5) * 3.2;
      y = j * 3.2 * sin(60);
      if (sqrt(x * x + y * y) <= grille_r) translate([spk_c[0] + x, spk_c[1] + y, -1]) cylinder(d = 2.0, h = floor_t + 2, $fn = 16);
    }
    translate([entry_x - dir * eps, bore_y, bore_z]) rotate([0, dir * 90, 0]) {
      rotate([0, 0, usb_on_right ? 0 : 180]) linear_extrude(bore_depth) teardrop2d(bore_d / 2);
      cylinder(d1 = bore_d + 1.6, d2 = bore_d, h = 0.8);
    }
    translate([entry_x, bore_y, bore_z]) scale([1, 1, 1.3]) sphere(d = 10);
    for (x = [out_l * 0.18, out_l * 0.82], y = [14, out_w - 14]) translate([x, y, -eps]) cylinder(d = feet_d, h = feet_depth);
  }
  if (stylus_rib > 0)
    translate([usb_on_right ? 6 : out_l - 12, bore_y - 0.4, bore_z - bore_d / 2 - eps]) cube([6, 0.8, stylus_rib + eps]);
}

// ---------------------------------------------------------------- tray (local module coords, z = 0 at tray underside)
module clip(L, side) {   // side 0 = J1 edge (W=0), 1 = far edge (W=mod_w)
  zt = tray_floor + ledge_h + mod_t;      // PCB top
  w0 = side == 0 ? -0.2 - clip_t : mod_w + 0.2;
  translate([L - clip_l / 2, 0, 0]) {
    translate([0, w0, 0]) cube([clip_l, clip_t, zt + 1.2]);
    // hook with a lead-in chamfer
    hull() {
      translate([0, side == 0 ? -0.2 : mod_w - hook, zt + 0.05]) cube([clip_l, hook + 0.2, 0.4]);
      translate([0, side == 0 ? -0.2 : mod_w + 0.1, zt + 1.15]) cube([clip_l, 0.1, 0.05]);
    }
  }
}
module tray() {
  zt = tray_floor + ledge_h;              // PCB underside
  difference() {
    union() {
      // floor + ears
      translate([-2.5, -2.4, 0]) cube([mod_l + 5, mod_w + 4.8, tray_floor]);
      for (e = tray_ears) hull() {
        translate([e[0], e[1], 0]) cylinder(d = 7.5, h = tray_floor + 0.8);
        translate([min(max(e[0], 0), mod_l), min(max(e[1], 2), mod_w - 2), 0]) cylinder(d = 4, h = tray_floor);
      }
      // edge ledges (tails sit between them)
      for (w = [0, mod_w - 1.2]) translate([1, w, 0]) cube([mod_l - 2, 1.2, zt]);
      // cross ribs at the ends
      for (l = [0.3, mod_l - 1.5]) translate([l, 0, 0]) cube([1.2, mod_w, zt]);
      // end stops: J5 end up to just under the socket, J3 end below the plugs
      translate([-1.3, 2, 0]) cube([1.0, mod_w - 4, zt + mod_t - 0.2]);
      translate([mod_l + 0.3, 2, 0]) cube([1.0, mod_w - 4, zt + 0.7]);
      // snap clips
      for (L = [6, 44]) clip(L, 0);
      for (L = [6, 36]) clip(L, 1);
    }
    for (e = tray_ears) translate([e[0], e[1], -1]) cylinder(d = m3_clear, h = 5);
    // lighten / wire pass-through
    translate([8, 3, -1]) cube([mod_l - 16, mod_w - 6, tray_floor + 2]);
  }
}

// ---------------------------------------------------------------- bezel
module front_openings(h) {
  wx = bx(view_u, view_l) - window_margin;
  wy = by(view_v, view_w) - window_margin;
  wl = view_l + 2 * window_margin;
  ww = view_w + 2 * window_margin;
  hull() {
    translate([wx, wy, z_front_in - 1]) cube([wl, ww, 1 + eps]);
    translate([wx - front_t, wy - front_t, out_h - eps]) cube([wl + 2 * front_t, ww + 2 * front_t, eps * 2]);
  }
  // J1 well with a small chamfer
  in_mod(z_front_in - 1) hull() {
    mrect(well, 1 + eps);
    translate([0, 0, h]) mrect([well[0] - 0.8, well[1] - 0.8, well[2] + 0.8, well[3] + 0.8], eps);
  }
}

module bezel() {
  difference() {
    union() {
      difference() {
        translate([0, 0, z_split]) rbox(out_l, out_w, out_h - z_split, corner_r);
        cavity(z_split - 1, rim_h + 1);
        translate([0, 0, z_split - 1]) difference() {
          translate([-1, -1, 0]) cube([out_l + 2, out_w + 2, lap + 1.1]);
          translate([0, 0, -1]) rbox_inset(wall / 2 + 0.1, lap + 4);
        }
      }
      for (p = concat(pcb_holes, cs_holes)) translate([p[0], p[1], z_split]) cylinder(d = pillar_d, h = rim_h + eps);
      // stops that keep the module from lifting when a connector is pulled off
      for (q = holddowns) { p = mpos(q[0], q[1]); translate([p[0], p[1], z_mod_top + 0.2]) cylinder(d = 2.6, h = z_front_in - z_mod_top - 0.2 + eps); }
      in_mod(z_mod_top + j3_housing_h + 0.2) mrect(j3_pad, z_front_in - z_mod_top - j3_housing_h - 0.2 + eps);
    }
    for (p = concat(pcb_holes, cs_holes)) translate([p[0], p[1], z_split - eps]) cylinder(d = m3_pilot, h = pilot_depth);
    usb_cut(z_split - 2, z_pcb_bot + usb_z + usb_cut_h / 2);
    front_openings(front_t + 1);
    // legend under the J1 well
    for (i = [0 : 8]) {
      p = mpos(j1_L(i), -well_margin - 2.6);
      lbl = j1_label(i);
      translate([p[0], p[1], out_h - 0.6]) linear_extrude(1)
        text(lbl, size = lbl == "INT" ? 1.5 : 2.2, font = font, halign = "center", valign = "center");
    }
    // title to the right of the well
    tp = mpos(well[2] + (out_l - wall - mod_x0 - well[2]) / 2, mod_w / 2);
    translate([tp[0], tp[1], out_h - 0.6]) linear_extrude(1) {
      translate([0, 2.6]) text("MEMBRANE", size = 3.2, font = font, halign = "center", valign = "center");
      translate([0, -2.6]) text("TESTER", size = 3.2, font = font, halign = "center", valign = "center");
    }
  }
}

module fit_test() {
  t = 1.2;
  difference() {
    rbox(out_l, out_w, t, corner_r);
    translate([0, 0, -(z_front_in - 1) - 0.5]) front_openings(10);
    for (p = concat(pcb_holes, cs_holes)) translate([p[0], p[1], -1]) cylinder(d = hole_d, h = t + 2);
    translate([0, 0, t - 0.4]) linear_extrude(1) {
      difference() { translate([pcb_x0, pcb_y0]) square([pcb_l, pcb_w]); translate([pcb_x0 + 0.5, pcb_y0 + 0.5]) square([pcb_l - 1, pcb_w - 1]); }
      difference() { translate([bx(glass_u, glass_l), by(glass_v, glass_w)]) square([glass_l, glass_w]);
                     translate([bx(glass_u, glass_l) + 0.5, by(glass_v, glass_w) + 0.5]) square([glass_l - 1, glass_w - 1]); }
      translate([mod_x0, mod_y0]) difference() { square([mod_l, mod_w]); translate([0.5, 0.5]) square([mod_l - 1, mod_w - 1]); }
    }
    translate([usb_on_right ? out_l - 3 : -1, by(usb_v) - usb_cut_w / 2, -1]) cube([4, usb_cut_w, t + 2]);
  }
}

// ---------------------------------------------------------------- mock-ups
module hw171() {
  color("navy") cube([mod_l, mod_w, mod_t]);
  color("yellow") translate([j1_L(0) - 1.27, j1_w - 1.27, mod_t]) cube([9 * j1_pitch, 2.54, 2.6]);
  color("gold") for (i = [0 : 8]) translate([j1_L(i) - 0.32, j1_w - 0.32, mod_t]) cube([0.64, 0.64, 8.3]);
  color("yellow") translate([14, 5.8, mod_t]) cube([4, 8.5, 7.0]);
  color("black") translate([22, 5.5, mod_t]) cube([9.5, 9, 1.7]);
  color("black") translate([-j5_overhang, 2.8, mod_t]) cube([6, 10, 2.0]);                   // J5
  color("black") translate([44.5, 3.8, mod_t]) cube([3.2, 10, j3_housing_h]);                 // J3
  color("#222") translate([mod_l, 3.8, mod_t - 0.7]) cube([j3_reach - 0.5, 10, plug_h]);     // Dupont
  for (k = [0 : 3]) color(["red", "black", "yellow", "blue"][k])
    translate([mod_l + j3_reach - 0.5, 5 + k * 2.54, mod_t + 0.5]) rotate([0, 90, 0]) cylinder(d = 1.2, h = 3, $fn = 8);
}
module mockups() {
  color("seagreen") difference() {
    translate([pcb_x0, pcb_y0, z_pcb_bot]) cube([pcb_l, pcb_w, pcb_t]);
    for (p = pcb_holes) translate([p[0], p[1], z_pcb_bot - 1]) cylinder(d = hole_d, h = 4);
  }
  color("dimgray") translate([bx(glass_u, glass_l), by(glass_v, glass_w), z_split]) cube([glass_l, glass_w, glass_t]);
  color("steelblue") translate([bx(view_u, view_l), by(view_v, view_w), z_split + glass_t - 0.1]) cube([view_l, view_w, 0.15]);
  color("silver") translate([usb_on_right ? pcb_x0 + pcb_l - 7 : pcb_x0 - 1, by(usb_v) - 4.5, z_pcb_bot + usb_z - 1.6]) cube([8, 9, 3.2]);
  in_mod(z_mod_bot) hw171();
  color("orange") in_mod(z_tray) tray();
  color("black") translate([spk_c[0], spk_c[1], floor_t]) cylinder(d = spk_d, h = spk_t);
  color("white") translate([usb_on_right ? -stylus_protrude : out_l - stylus_len + stylus_protrude, rail / 2, z_split / 2])
    rotate([0, 90, 0]) cylinder(d = stylus_d, h = stylus_len);
}

// ---------------------------------------------------------------- output
if (part == "base") base();
else if (part == "bezel") translate([0, out_w, out_h]) rotate([180, 0, 0]) bezel();
else if (part == "tray") tray();
else if (part == "none") { }
else if (part == "fit_test") translate([0, out_w, 1.2]) rotate([180, 0, 0]) fit_test();
else if (part == "print_layout") {
  base();
  translate([0, out_w * 2 + 10, out_h]) rotate([180, 0, 0]) bezel();
  translate([out_l + 15, 10, 0]) tray();
} else {
  color("#3a3f47") base();
  if (show_parts) mockups();
  color("#555c66", 0.85) translate([0, 0, explode]) bezel();
}

// all values in mm
// thickness of metal in mm
case_wall_thickness = 2;
// the "wings" on the bottom of the case need to be thicker than the other walls.
edge_wall_thickness = case_wall_thickness + 1;

// X
case_width = 140;
// Y
case_thickness = 70;
// Z
case_height = 50;

// What percent of case height should the front vertical face take up?
// (or, at what height should we start the angled face)
case_front_face_height_factor = 0.3;
case_front_face_height = case_height * case_front_face_height_factor;
case_screw_distance_from_edge = case_width * 0.05;
case_screw_distance_from_bottom = case_front_face_height / 2;
// button holes
button_sides = 14;

// Dimensions for a USB type B port: http://www.molex.com/pdm_docs/sd/670687041_sd.pdf
case_usb_type_b_port_side = 12;

// OLED panels
oled_screen_sides = 34;
oled_panel_width = 34;
oled_distance_from_side = 15;

// extra space to make the objects larger than what we subtract from
trim = 5;


oled_panel_height = 34;
oled_screw_to_screw_width = 39;
oled_screw_to_screw_height = 28; 
oled_screw_hole_diameter = 3;


// creates a solid object in the shape of the chess clock.
module case_solid(width, thickness, height, front_face_height)
{
  extra_width_for_trimming = 2;
  color("white") {
   difference() {
      cube([width, thickness, height]);
      // create a cube that is cut away from the main block to form the
      // slanted front of the case.
      translate([-extra_width_for_trimming / 2, 0, front_face_height]) {
        color("red") {
          // The cube we subtract needs to be wider than the base cube
          // so we shift our x origin -extra_width_for_trimming/2 over and
          // extra_width_for_trimming to the width of the cube
          // We need to start the cube up on the z origin; we want this origin
          // to be 1/5 of the total case height.
          // To see this in faux-debug mode, comment out the difference() call above.
          rotate([45,0,0]) {
            cube([width + extra_width_for_trimming, 
                  thickness, 
                  height]);
          }
        }
      }
    }
  }
}


module cherry_mx_switch_hole(switch_side_length, wall_thickness) {
  color("black") {
    cube([switch_side_length, switch_side_length, wall_thickness * 2]);
  }
}


module top_buttons(width, thickness, height, wall_thickness, 
                   button_side_length) {
  // the buttons are positioned 1/10 the width of the case from the side,
  // and 2/3 of the distance of the thickness of the case from the front.
  button_distance_from_side = width / 10;
  button_distance_from_front = thickness * 0.66;
  // Far right button
  translate([width - button_side_length - button_distance_from_side,  
             button_distance_from_front, 
             case_height - wall_thickness * 2]) {
    cherry_mx_switch_hole(button_side_length, wall_thickness * 2);
  }
  // Far left button
  translate([button_distance_from_side,  
             button_distance_from_front, 
             case_height - wall_thickness * 2]) {
    cherry_mx_switch_hole(button_side_length, wall_thickness * 2);
  }
  // Middle button
  translate([width / 2 - button_side_length/2,  
             button_distance_from_front, 
             case_height - wall_thickness * 2]) {
    cherry_mx_switch_hole(button_side_length, wall_thickness * 2);
  }     
}

module screw_hole(diameter, height) {
  // The native cylinder has 8 sides, which stinks, this setting
  // adds more sides to the cylinder.
  hole_resolution = 100;
  cylinder(r=diameter / 2, h = height, $fn = hole_resolution);
}

module oled_screw_holes(hole_diameter, panel_side, wall_thickness) { 
    wall_thickness = wall_thickness * 2;
    translate([-hole_diameter, hole_diameter, 0]) 
      screw_hole(hole_diameter, wall_thickness);
    translate([panel_side + hole_diameter, hole_diameter, 0])
      screw_hole(hole_diameter, wall_thickness);
    translate([-hole_diameter, panel_side - hole_diameter, 0]) 
      screw_hole(hole_diameter, wall_thickness);
    translate([panel_side + hole_diameter, panel_side - hole_diameter, 0]) 
      screw_hole(hole_diameter, wall_thickness);
}

module oled_screens(width, thickness, height, wall_thickness, panel_side_length) 
{
  distance_from_side = width / 10;
  distance_from_front = thickness / 4;
  distance_from_bottom = height / 7;
  
  // TODO(adamf) make this a translate/rotate, not rotate/translate, this will let
  // us clear up the meaning of the factors above.
  rotate([45, 0, 0]) {
    // Far right panel
    translate([width - panel_side_length - distance_from_side, 
               distance_from_front, 
               distance_from_bottom]) {
      color("purple") cube([panel_side_length, panel_side_length, wall_thickness * 2]);
      oled_screw_holes(oled_screw_hole_diameter, panel_side_length, wall_thickness);
    }
    
    // Far left panel
    translate([distance_from_side, 
               distance_from_front, 
               distance_from_bottom]) {
      color("purple") cube([panel_side_length, panel_side_length, wall_thickness * 2]);
      oled_screw_holes(oled_screw_hole_diameter, panel_side_length, wall_thickness);
    }
  }
}

module case_screw_holes(width, thickness, wall_thickness,
                            screw_height_from_bottom, screw_distance_from_edge)
{
  hole_diameter = oled_screw_hole_diameter;
  // Make the screw hole depth long enough to work with the top and bottom.
  screw_hole_depth = wall_thickness * 6;

  // front left
  translate([screw_distance_from_edge, 
             wall_thickness * 2, 
             screw_height_from_bottom])
    rotate([90, 0, 0]) 
      screw_hole(hole_diameter, screw_hole_depth);
  // front right
  translate([width - screw_distance_from_edge, 
             wall_thickness * 2, 
             screw_height_from_bottom])
    rotate([90, 0, 0]) 
      screw_hole(hole_diameter, screw_hole_depth);  
  
  // rear left
  translate([screw_distance_from_edge, 
             case_thickness + wall_thickness, 
             screw_height_from_bottom])
    rotate([90, 0, 0]) 
      screw_hole(hole_diameter, screw_hole_depth);
      
  // rear right
  translate([width - screw_distance_from_edge, 
             case_thickness + wall_thickness, 
             screw_height_from_bottom])
    rotate([90, 0, 0]) 
      screw_hole(hole_diameter, screw_hole_depth);      
}

module case_top(width, thickness, height, wall_thickness,
                front_face_height,
                button_side_length = button_sides, 
                panel_side_length = oled_screen_sides,
                screw_distance_from_bottom = case_screw_distance_from_bottom,
                screw_distance_from_edge = case_screw_distance_from_edge)
{
  
  difference() {
    // create the main case solid
    case_solid(width, thickness, height, front_face_height);
    
    // remove a smaller, but same shape solid, leaving the core case top
    translate([-wall_thickness, wall_thickness, -wall_thickness]) {
      case_solid(width + wall_thickness * 2, 
                 thickness - wall_thickness * 2, 
                 height, front_face_height);
    }
    // Remove the buttons holes
    top_buttons(width, thickness, height, wall_thickness, 
                button_side_length);  
    
    // Remove the OLED screens & their screw holes
    oled_screens(width, thickness, height, wall_thickness, panel_side_length);
    
    // Remove the front and back screw holes
    case_screw_holes(width, thickness, wall_thickness,
                     screw_distance_from_bottom, screw_distance_from_edge);
    
  }
}


module usb_port(width, thickness, height, wall_thickness,
                usb_port_side_length=case_usb_type_b_port_side) {
  translate([-wall_thickness / 2, thickness * 0.66, height / 5])
    rotate([90, 0, 0])
                cube([wall_thickness * 2, usb_port_side_length, usb_port_side_length]);
  
  
}

module case_bottom(width, thickness, height, wall_thickness,
                   front_face_height,
                   screw_distance_from_bottom = case_screw_distance_from_bottom,
                   screw_distance_from_edge = case_screw_distance_from_edge) 
{
 difference() {
    case_solid(width, thickness, height, front_face_height);
    translate([wall_thickness, wall_thickness, wall_thickness]) {
      case_solid(width - wall_thickness * 2, 
                 thickness - wall_thickness * 2, 
                 height + wall_thickness * 2, 
                 front_face_height);
    }
    translate([wall_thickness, 0, front_face_height]) {
      cube([width - wall_thickness * 2,
            thickness + wall_thickness,
            height]);
    }
    case_screw_holes(width, thickness, wall_thickness,
                     screw_distance_from_bottom, screw_distance_from_edge);
    usb_port(width, thickness, height, wall_thickness);
  }
}


// Rotate the top piece for easier printing.
rotate([0, -90, -90])
  case_top(case_width, case_thickness, case_height, case_wall_thickness,
           case_front_face_height);

// Leave the bottom piece flat.
translate([case_width, 0, 0])
  case_bottom(case_width, 
              case_thickness - (case_wall_thickness * 2), 
              case_height - case_wall_thickness, 
              case_wall_thickness,
              case_front_face_height - case_wall_thickness);    
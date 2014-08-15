// all values in mm
// thickness of metal in mm
wall_thickness = 1;
// X
case_width = 140;
// Y
case_thickness = 70;
// Z
case_height = 50;

// button holes
button_sides = 14;
button_distance_from_side = 15;
button_distance_from_front = 45;
// OLED panels
oled_screen_sides = 34;
oled_panel_width = 45;
oled_panel_height = 34;
oled_screw_to_screw_width = 39;
oled_screw_to_screw_height = 28; 
oled_screw_hole_diameter = 4;
oled_distance_from_side = 10;

// extra space to make the objects larger than what we subtract from
trim = 5;


/*
// reference planes
translate([0, 0, -10]) {
 cube([case_width, case_thickness, 1]);
}
*/

// the top

rotate([0,-90,-90])
color("white")
difference() {

// external shell
difference() {
cube([case_width, case_thickness, case_height]);
  rotate([45,0,0]) {
    color("red") {
      translate([-1,9,10]) cube([case_width + 2, case_thickness + 2, case_height]);
    }
  }
}


// subtract a copy of the shell from the inside
translate([-wall_thickness,wall_thickness,-wall_thickness]) {
  difference() {
    color("green") cube([case_width + trim, case_thickness - wall_thickness - wall_thickness, case_height]);
    rotate([45,0,0]) {
      color("blue") {
        translate([-1,9,10]) cube([case_width + trim + 2, case_thickness - 18, 25]);
      }
    }
  }
}


// button holes
translate([case_width - button_sides - button_distance_from_side, button_distance_from_front, case_height - 3]) {
  color("black") cube([button_sides, button_sides, wall_thickness + trim]);
}
translate([button_distance_from_side, button_distance_from_front, case_height - 3]) {
  color("black") cube([button_sides, button_sides, wall_thickness + trim]);
}


rotate([45,0,0]) {
  translate([case_width - oled_panel_width - oled_distance_from_side, 18, 0]) {
    color("purple") cube([oled_panel_width, oled_panel_height, 30]);
    translate([-oled_screw_hole_diameter,oled_screw_hole_diameter,0]) cylinder(r=oled_screw_hole_diameter/2, h=20, $fn=100);
	translate([oled_panel_width + oled_screw_hole_diameter,oled_screw_hole_diameter,0]) cylinder(r=oled_screw_hole_diameter/2, h=20, $fn=100);
	translate([-oled_screw_hole_diameter,oled_panel_height - oled_screw_hole_diameter,0]) cylinder(r=oled_screw_hole_diameter/2, h=20, $fn=100);
	translate([oled_panel_width + oled_screw_hole_diameter,oled_panel_height - oled_screw_hole_diameter,0]) cylinder(r=oled_screw_hole_diameter/2, h=20, $fn=100);
  }

  translate([oled_distance_from_side, 18, 0]) {
    color("purple") cube([oled_panel_width, oled_panel_height, 30]);
    translate([-oled_screw_hole_diameter,oled_screw_hole_diameter,0]) cylinder(r=oled_screw_hole_diameter/2, h=20, $fn=100);
	translate([oled_panel_width + oled_screw_hole_diameter,oled_screw_hole_diameter,0]) cylinder(r=oled_screw_hole_diameter/2, h=20, $fn=100);
	translate([-oled_screw_hole_diameter,oled_panel_height - oled_screw_hole_diameter,0]) cylinder(r=oled_screw_hole_diameter/2, h=20, $fn=100);
	translate([oled_panel_width + oled_screw_hole_diameter,oled_panel_height - oled_screw_hole_diameter,0]) cylinder(r=oled_screw_hole_diameter/2, h=20, $fn=100);
  }

}
 

rotate([90,0,0]) {
	translate([7, 7, -5]) cylinder(r=oled_screw_hole_diameter/2, h=20, $fn=100);
    translate([case_width - 7, 7, -5]) cylinder(r=oled_screw_hole_diameter/2, h=20, $fn=100);

   translate([7, 7, -case_thickness - 5]) cylinder(r=oled_screw_hole_diameter/2, h=20, $fn=100);
   translate([case_width - 7, 7, -case_thickness - 5]) cylinder(r=oled_screw_hole_diameter/2, h=20, $fn=100);


}

}

// the bottom


// shift everthing over
color("white")
translate([180,1,0]) {

	difference() {
		difference() {
		  cube([case_width, case_thickness - 2, case_height - 1]);
		  rotate([45,0,0]) {
		    color("red") {
		      translate([-1,9,10]) cube([case_width + 2, case_thickness + 2, case_height]);
		    }
		  }
		}

		translate([1,1,1]) {
			color("orange") cube([case_width - 2, case_thickness - 4, case_height + trim]);
             translate([0,-trim,12]) cube([case_width-2, case_thickness + trim + trim, case_height + trim]);
        }

		rotate([90,0,0]) {
			translate([7, 7, -5]) cylinder(r=oled_screw_hole_diameter/2, h=20, $fn=100);
		    translate([case_width - 7, 7, -5]) cylinder(r=oled_screw_hole_diameter/2, h=20, $fn=100);
		
		   translate([7, 7, -case_thickness - 5]) cylinder(r=oled_screw_hole_diameter/2, h=20, $fn=100);
		   translate([case_width - 7, 7, -case_thickness - 5]) cylinder(r=oled_screw_hole_diameter/2, h=20, $fn=100);
		
		
		}

	}
}

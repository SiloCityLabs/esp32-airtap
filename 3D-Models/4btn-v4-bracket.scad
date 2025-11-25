$fn = 25;

cornerRadius = 3;

module roundedSquare(size = [10, 10], radius = 1) {
  hull() {
    for (x = [radius, size[0] - radius])
      for (y = [radius, size[1] - radius])
        translate([x, y]) circle(r=radius);
  }
}

module roundedPlate(width, height, thickness, radius) {
  linear_extrude(thickness)
    roundedSquare([width, height], radius);
}

// 37.8mm Width x 92mm Height x 3mm Thickness

bracketWidth = 37.8;
bracketHeight = 92;
bracketThickness = 1.97;
bracketHoleDiameter = 6.1;
m3HoleDiameter = 3.3;
buttonHoleDiameter = 4.85;
oledWidth = 26;
oledHeight = 14.5;

lockingHoleWidth = 2;
lockingHoleHeight = 4;

difference() {
  roundedPlate(bracketWidth, bracketHeight, bracketThickness, cornerRadius);

  // Subtract LCD Hole
  translate([(bracketWidth - oledWidth) / 2, 15, -1]) {
    cube([oledWidth, oledHeight, bracketThickness + 2]);
  }

  //Subtract IR Hole
  translate([(bracketWidth - 9) / 2, 44, -1]) {
    cube([9, 8, bracketThickness + 2]);
  }

  //Subtract IR Hole Buffer
  translate([(bracketWidth - 14) / 2, 42, 0.75]) {
    cube([14, 12, bracketThickness + 2]);
  }

  //Subtract io header
  translate([(bracketWidth - 10) / 2, 9, 0.75]) {
    cube([10, 7, bracketThickness + 2]);
  }

  //Subtract LCD ribbon
  translate([(bracketWidth - oledWidth) / 2, 29, 0.75]) {
    cube([oledWidth, 7, bracketThickness + 2]);
  }

  //Subtract Screw holes (top)
  translate([4.35, 5, -1]) {
    cylinder(h=bracketThickness + 2, d=bracketHoleDiameter);
  }
  translate([bracketWidth - 4.35, 5, -1]) {
    cylinder(h=bracketThickness + 2, d=bracketHoleDiameter);
  }

  //Subtract Screw holes (bottom)
  translate([bracketWidth / 2, 82 + bracketHoleDiameter, -1]) {
    cylinder(h=bracketThickness + 2, d=bracketHoleDiameter);
  }

  //Button holes x4 62 then 70 from top
  translate([12, 61.5 + (buttonHoleDiameter / 2), -1]) {
    cylinder(h=bracketThickness + 2, d=buttonHoleDiameter);
  }
  translate([bracketWidth - 12, 61.5 + (buttonHoleDiameter / 2), -1]) {
    cylinder(h=bracketThickness + 2, d=buttonHoleDiameter);
  }

  translate([12, 68.5 + (buttonHoleDiameter / 2), -1]) {
    cylinder(h=bracketThickness + 2, d=buttonHoleDiameter);
  }
  translate([bracketWidth - 12, 68.5 + (buttonHoleDiameter / 2), -1]) {
    cylinder(h=bracketThickness + 2, d=buttonHoleDiameter);
  }

  //Subtract Locking Hole
  translate([6.7, 62, -1]) {
    cube([lockingHoleWidth, lockingHoleHeight, bracketThickness + 2]);
  }
  translate([bracketWidth - 8.7, 62, -1]) {
    cube([lockingHoleWidth, lockingHoleHeight, bracketThickness + 2]);
  }
  translate([6.7, 69, -1]) {
    cube([lockingHoleWidth, lockingHoleHeight, bracketThickness + 2]);
  }
  translate([bracketWidth - 8.7, 69, -1]) {
    cube([lockingHoleWidth, lockingHoleHeight, bracketThickness + 2]);
  }

  //Subtract alignment hole for locking hole
  translate([bracketWidth / 2 - 0.75, 63.75, -1]) {
    cylinder(h=bracketThickness + 2, d=2.25);
  }
  translate([bracketWidth / 2 + 0.75, 70.75, -1]) {
    cylinder(h=bracketThickness + 2, d=2.25);
  }

  //Subtract button area
  translate([4, 60, bracketThickness - 0.5]) {
    cube([30, 15, 1]);
  }
}

supportPinDiameter = 1.21;
supportPinHeight = 6;
supportPinBracketWidth = 0.93;
supportPinBracketHeight = 4.25;

//Add top support pin
translate([bracketWidth / 2, 3, 0]) {
  cylinder(h=supportPinHeight + bracketThickness, d=supportPinDiameter);
}
//Add top support bracket for pin
translate([(bracketWidth / 2) - (supportPinBracketWidth / 2), 1, 0]) {
  cube([supportPinBracketWidth, supportPinBracketHeight, supportPinBracketHeight + bracketThickness]);
}

//Add Bottom support pin
translate([bracketWidth - 3, 61 + (supportPinDiameter / 2), 0]) {
  cylinder(h=supportPinHeight + bracketThickness, d=supportPinDiameter);
}
//Add Bottom support bracket for pin
translate([bracketWidth - 3 - (supportPinBracketWidth / 2), 58, 0]) {
  cube([supportPinBracketWidth, 3, supportPinBracketHeight + bracketThickness]);
}
translate([0, 58, 0]) {
  cube([bracketWidth, supportPinBracketWidth, supportPinBracketHeight + bracketThickness]);
}

module screwBracket() {
  cylinder(h=1.7, d=8.75);
  cylinder(h=supportPinBracketHeight, d=5.6);
}

module screwBracketandHole() {
  difference() {
    screwBracket();
    translate([0, 0, -1]) {
      cylinder(h=10, d=3);
    }
  }
}
//screw mount holes
translate([4.35, 5, bracketThickness]) {
  difference() {
    screwBracketandHole();
    translate([0, 0, -1]) {
      cylinder(h=1.5, d=bracketHoleDiameter);
    }
  }
}

translate([bracketWidth - 4.35, 5, bracketThickness]) {
    difference() {
      screwBracketandHole();
      translate([0, 0, -1]) {
        cylinder(h=1.5, d=bracketHoleDiameter);
      }
    }
}

//Bottom 75 apart from top
translate([4.35, 80, bracketThickness]) {
  screwBracketandHole();
}
translate([bracketWidth - 4.35, 80, bracketThickness]) {
  screwBracketandHole();
}

buttonHoleSlider = buttonHoleDiameter + 1;
buttonHoleSliderOffsetHeight = bracketThickness - 0.5;
//Button holes x4 62 then 70 from top

translate([12, 61 + (buttonHoleSlider / 2), buttonHoleSliderOffsetHeight]) {
  difference() {
    cylinder(h=1, d=buttonHoleSlider);
    translate([0, 0, -1]) {
      cylinder(h=10, d=3.65);
    }
  }
}
translate([bracketWidth - 12, 61 + (buttonHoleSlider / 2), buttonHoleSliderOffsetHeight]) {
  difference() {
    cylinder(h=1, d=buttonHoleSlider);
    translate([0, 0, -1]) {
      cylinder(h=10, d=3.65);
    }
  }
}

translate([12, 68 + (buttonHoleSlider / 2), buttonHoleSliderOffsetHeight]) {
  difference() {
    cylinder(h=1, d=buttonHoleSlider);
    translate([0, 0, -1]) {
      cylinder(h=10, d=3.65);
    }
  }
}
translate([bracketWidth - 12, 68 + (buttonHoleSlider / 2), buttonHoleSliderOffsetHeight]) {
  difference() {
    cylinder(h=1, d=buttonHoleSlider);
    translate([0, 0, -1]) {
      cylinder(h=10, d=3.65);
    }
  }
}

$fn = 25;

cornerRadius = 3;

module roundedSquare(size=[10,10], radius=1) {
    hull() {
        for (x = [radius, size[0] - radius])
            for (y = [radius, size[1] - radius])
                translate([x, y]) circle(r = radius);
    }
}

module roundedPlate(width, height, thickness, radius) {
    linear_extrude(thickness)
        roundedSquare([width, height], radius);
}

// 37.8mm Width x 92mm Height x 3mm Thickness

bracketWidth = 37.8;
bracketHeight = 92;
bracketThickness = 0.98;
bracketHoleDiameter = 6.1;
m3HoleDiameter = 3.3;
buttonHoleDiameter = 3.85;
oledWidth = 31;
oledHeight = 45.5;

lockingHoleWidth = 2;
lockingHoleHeight = 3;

difference() {
    roundedPlate(bracketWidth, bracketHeight, bracketThickness, cornerRadius);

    // Subtract LCD Hole
    translate([(bracketWidth - oledWidth)/2, 12, -1]) {
        cube([oledWidth, oledHeight, bracketThickness+2]);
    }

    //Subtract Screw holes (top)
    translate([4.35, 5, -1]) {
        cylinder(h = bracketThickness+2, d = bracketHoleDiameter);
    }
    translate([bracketWidth - 4.35, 5, -1]) {
        cylinder(h = bracketThickness+2, d = bracketHoleDiameter);
    }

    //Subtract Screw holes (bottom)
    translate([bracketWidth/2, 82+bracketHoleDiameter, -1]) {
        cylinder(h = bracketThickness+2, d = bracketHoleDiameter);
    }

    //Subtract button area
    translate([4, 60, -1]) {
        cube([30, 15, 3]);
    }
}

irWidth = 27;
irHeight = 7.75;
irDepth = 2.5;

translate([(bracketWidth - irWidth)/2, 76.5, bracketThickness-0.5]) {
    roundedPlate(irWidth, irHeight, irDepth, 1);
}
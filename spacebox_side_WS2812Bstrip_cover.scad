include <BOSL2/std.scad>
$fn=100;

height = 2;

diff(){
  prismoid(size1=[167,21.5], size2=[167,20.5], h=height);
  
  //grid_copies(n=[16,2], spacing=10)
  //tag("remove") zcyl(d=8, h=20);
  
  tag("remove") {
    grid_copies(n=[5,1], spacing=33.3) {
      cuboid([5.1,5.1,10]);
      // Capacitor
      left(5.1/2) up(height-1.2) cuboid([3,2.8,1.2], anchor=BOTTOM+RIGHT);
      // Solder joints to 5050 SMD
       up(height-1.2) cuboid([5.1,5.1+3,1.2], anchor=BOTTOM);
    }
    // Remove material 
    // grid_copies(n=[4,1], spacing=33.3) zcyl(d=16, h=20);   
  }
}
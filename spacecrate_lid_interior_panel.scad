include <BOSL2/std.scad>

difference(){
 
  translate([-42,-45,0])
  linear_extrude(2)
  polygon(points = [[0,0], [84,0], [83,90], [1,90], [0,0]]);
  
  
 // prismoid(size1=[84,90], size2=[83.4,90], h=20);
  translate([-42,45])
  cuboid([40,4,10], anchor=BACK+LEFT);
  
  
//cuboid([84,89.6,1], anchor=BOTTOM);
  //tag("remove") 
  grid_copies(spacing=40, n=[2,2])
zcyl(d=28,h=10, anchor=BOTTOM);
}
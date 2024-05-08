include <BOSL2/std.scad>

diff(){

cuboid([152,3,25]) {
  attach(TOP) prismoid(size1=[152,3], size2=[129,3], h=12.5);
  up(6) attach(BACK) rect_tube(isize=[128.6,32], wall=1, h=5);
}
tag("remove")up(6)cuboid([128.6,8,32]);
   
}
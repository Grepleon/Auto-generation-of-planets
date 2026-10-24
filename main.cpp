#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "draw.cpp"
#include "objects/planet.cpp"
#include "objects/sun.cpp"
#include "objects/scene.cpp"
#include <vector>

using namespace std;

int main() {
    int x = 2560;
    int y = 1440;
    Drawer draw = Drawer(x, y);
    draw.blackRectangle(0, 0, x-1, y-1);
    Camera cam(2560, 1440, Vec3(0, 0, -60), 40);
    Scene scene(cam);

    Sun sun(1024, 512, 1, draw, Vec3(-12, 2, 20), 8);
    sun.fill();
    sun.set_pose(0.3, 0.1);          // rot, tilt

    Planet earth(1024, 512, 1, draw, Vec3(10, -2, 5), 3);
    earth.fill();
    earth.set_pose(1.2, 0.4);

    Planet mars(1024, 512, 1, draw, Vec3(-1, -12, 1), 2);
    mars.fill();
    mars.set_pose(1.2, 0.4);

    Sun sun2(1024, 512, 1, draw, Vec3(-20, -5, 36), 8);
    sun2.fill();
    sun2.set_pose(0.3, 0.1);  

    scene.add(&sun);
    scene.add(&sun2);
    scene.add(&earth);
    scene.add(&mars);
    scene.render();
    draw.Save("data/sun/sun-1.4.png");
}
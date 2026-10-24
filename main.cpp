#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "draw.cpp"
#include "objects/planet.cpp"
#include "objects/sun.cpp"
#include "objects/blackhole.cpp"
#include "objects/scene.cpp"
#include <vector>

using namespace std;

int main() {
    int x = 2560;
    int y = 1440;
    Drawer draw = Drawer(x, y);
    draw.blackRectangle(0, 0, x-1, y-1);
    Camera cam(2560, 1440, Vec3(0, 0, -60), 40);
    Scene scene(cam, draw);

    BlackHole bh(1024, 512, 1, draw, Vec3(0, 0, 40), 6);   // радиус горизонта = 2
    bh.fill();
    bh.set_pose(0.0, 0.2);

    BlackHole bh2(1024, 512, 1, draw, Vec3(0, 10, 20), 3);   // радиус горизонта = 2
    bh2.fill();
    bh2.set_pose(0.0, 0.2);

    scene.add(&bh);
    scene.render();
    draw.Save("data/sun/sun-3.3.png");
}
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "draw.cpp"
#include "objects/planet.cpp"
#include <vector>

using namespace std;

int main() {
    Drawer draw = Drawer(2560, 1440);
    draw.blackRectangle(0, 0, 2560-1, 1440-1);
    Planet planet = Planet(2560, 1440, 1, draw);
    planet.fill();
    planet.show3D(500, 0, 0.5);

    Planet planet1 = Planet(2560, 1440, 1, draw);
    planet1.fill();
    planet1.show3D(1250, 100, 0.3);

    draw.Save("data/planet/planet-6.0.png");
}
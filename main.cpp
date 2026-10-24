#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "draw.cpp"
#include "objects/planet.cpp"
#include <vector>

using namespace std;

int main() {
    Drawer draw = Drawer(10000, 10000);
    draw.blackRectangle(0, 0, 10000-1, 10000-1);
    Planet planet = Planet(10000, 10000, 1, draw);
    planet.fill();
    planet.show3D(0, 0);

    draw.Save("data/planet/planet-8.5.png");
}
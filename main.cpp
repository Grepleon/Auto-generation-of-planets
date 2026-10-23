#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "draw.cpp"
#include "objects/planet.cpp"
#include <vector>

using namespace std;

int main() {
    Drawer draw = Drawer(10000, 1440);
    draw.blackRectangle(0, 0, 2560-1, 1440-1);
    Planet planet = Planet(2560, 1440, 1, draw);
    planet.fill();
    for (int i=0; i<30; i++)
        planet.show3D(-400 + i * 500, 0, 0.3, i, i / 60.0);


    draw.Save("data/planet/planet-7.3f.png");
}
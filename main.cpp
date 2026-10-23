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
    planet.show();
    draw.Save("data/planet/planet-3.4.png");
}
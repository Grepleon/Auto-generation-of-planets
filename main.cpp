#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "draw.cpp"
#include "objects/planet.cpp"
#include <vector>

using namespace std;

int main() {
    Drawer draw = Drawer(2560, 1440);
    
    Planet planet = Planet(100, 100, 5, draw);
    planet.show();
    draw.Save("data/planet/planet-0.png");
}
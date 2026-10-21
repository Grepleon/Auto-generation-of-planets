#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "draw.cpp"
#include <vector>

using namespace std;

int main() {
    Draw draw = Draw(2560, 1440);
    draw.blackRectangle(0, 500, 1000, 1000);
    draw.circle(500, 500, 100, new int[] {225, 0, 0});
    draw.oval(1000, 1000, 2000, 1200, new int[] {0, 255, 0});
    draw.Save("data/oval-1.png");
}
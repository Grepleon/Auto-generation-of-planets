#include "draw.cpp"
#include <vector>
#include <string>
#include <cmath>

using namespace std;

class Cell{
    private:
        int r, g, b;
    public:
        Cell(int r, int g, int b){
            this->r = r;
            this->g = g;
            this->b = b;
        }

        int* get_color(){
            return new int[] {r, g, b};
        }
};

struct Material {
    Cell soil = Cell(15, 245, 45);
    Cell water = Cell(0, 20, 240);
};

class Planet{
    private:
        int x, y, dx;
        vector<vector<Cell>> cells;
        Material material;
        Drawer draw;
    
    public:
        Planet(int x, int y, int dx, Drawer draw): x(x), y(y), dx(dx), draw(draw)
        {
            cells.resize(y, vector<Cell>(x, material.soil));
        }

        void show(){
            for (int iy=0; iy < y; iy++){
                for (int ix=0; ix < x; ix++){
                    draw.rectangle(ix * dx, iy * dx, ix * dx + dx, iy * dx + dx, cells[iy][ix].get_color());
                }   
            }
        }
};
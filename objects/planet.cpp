#include "draw.cpp"
#include "perlin.cpp"
#include "cell.cpp"
#include <vector>
#include <string>
#include <cmath>

using namespace std;

struct Material {
    Cell soil = Cell(15, 245, 45);
    Cell water = Cell(0, 20, 240);
};

class Planet{
    private:
        int x, y, dx;
        vector<vector<Cell>> cells;
        Material material;
        Drawer& draw;
    
    public:
        Planet(int x, int y, int dx, Drawer& draw): x(x), y(y), dx(dx), draw(draw)
        {
            cells.resize(y, vector<Cell>(x, Material().soil));
        }

        void fill(){
            Perlin per = Perlin(x, y);
            per.fill(7, 0.5, 6);
            for (int i=0; i<x; i++){
                for (int j=0; j<y; j++){
                    if (per.get_cell(i, j) > 0.5){
                        cells[j][i] = Material().soil;
                    }
                    else {
                        cells[j][i] = Material().water;
                    }
                }
            }
        }

        void show(){
            for (int iy=0; iy < y; iy++){
                for (int ix=0; ix < x; ix++){
                    draw.rectangle(ix * dx, iy * dx, ix * dx + dx, iy * dx + dx, cells[iy][ix].get_color());
                }   
            }
        }
};
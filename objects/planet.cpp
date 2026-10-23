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
    Cell montain = Cell(192, 199, 201);

};

class Planet{
    private:
        int x, y, dx;
        vector<vector<Cell>> cells;
        Material material;
        Drawer& draw;

        int dist(int x1, int y1){
            int dx = x1 - x / 2;
            int dy = y1 - y / 2;
            return sqrt(dx * dx + dy * dy);
        } 
    
    public:
        Planet(int x, int y, int dx, Drawer& draw): x(x), y(y), dx(dx), draw(draw)
        {
            cells.resize(y, vector<Cell>(x, Material().soil));
        }

        void fill(){
            Perlin per = Perlin(x, y);
            per.fill(7, 0.5, 6);
            Perlin mon = Perlin(x, y);
            per.fill(7, 0.5, 6);
            for (int i=0; i<x; i++){
                for (int j=0; j<y; j++){
                    if (per.get_cell(i, j) > 0.5){
                        if (mon.get_cell(i, j) > 0.5) 
                            cells[j][i] = Material().montain;
                        else
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
                    draw.rectangle(
                        ix * dx,
                        iy * dx,
                        ix * dx + dx,
                        iy * dx + dx, 

                        cells[iy][ix].get_color());
                }   
            }
        }

        void show3D(int ddx=0, int ddy=0, double dr=1, double rot = 0.0, double tilt = 0.0){
    const double PI = acos(-1.0);

    int size = y * dx;                 // диаметр шара в пикселях
    double R  = size / 2.0 * dr;
    double cx = size / 2.0 + ddx;
    double cy = size / 2.0 + ddy;
    double ct = cos(tilt), st = sin(tilt);

    for (int py = 0; py < size + ddy; py += dx){
        for (int px = 0; px < size + ddx; px += dx){
            // центр блока в координатах единичного круга
            double u =  (px + dx / 2.0 - cx) / R;
            double v = -(py + dx / 2.0 - cy) / R;      // ось Y вверх
            double r2 = u * u + v * v;
            if (r2 > 1.0) continue;                     // вне шара

            double w = sqrt(1.0 - r2);                  // глубина к зрителю

            // наклон оси планеты (вращение вокруг X)
            double v2 = v * ct - w * st;
            double w2 = v * st + w * ct;

            // широта и долгота точки
            double lat = asin(v2);                      // -PI/2 .. PI/2
            double lon = atan2(u, w2) + rot;            // вращение планеты

            // в индексы карты
            double fx = fmod(lon / (2 * PI) + 0.5, 1.0);
            if (fx < 0) fx += 1.0;
            int ix = min(x - 1, (int)(fx * x));
            int iy = min(y - 1, max(0, (int)((0.5 - lat / PI) * y)));

            draw.rectangle(px, py, px + dx, py + dx, cells[iy][ix].get_color());
        }
    }
}
};
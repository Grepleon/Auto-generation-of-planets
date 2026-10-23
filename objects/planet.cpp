#include "draw.cpp"
#include "perlin.cpp"
#include "cell.cpp"
#include <vector>
#include <string>
#include <cmath>

using namespace std;

struct Material {
    // вода: от берега к глубине
    Cell shore      = Cell(40, 120, 235);
    Cell water      = Cell(0, 20, 240);
    Cell deep_water = Cell(0, 10, 150);
    Cell ice        = Cell(215, 235, 250);

    // суша: сухие и влажные биомы, от холодного к жаркому
    Cell tundra     = Cell(140, 150, 125);
    Cell steppe     = Cell(165, 195, 60);
    Cell desert     = Cell(225, 190, 105);
    Cell taiga      = Cell(25, 95, 70);
    Cell forest     = Cell(15, 190, 45);
    Cell jungle     = Cell(5, 110, 35);

    Cell sand       = Cell(230, 215, 140);
    Cell rock       = Cell(120, 125, 130);
    Cell snow       = Cell(240, 245, 250);
};

static inline double clamp01(double v){
    return v < 0 ? 0 : (v > 1 ? 1 : v);
}

static inline double sstep(double e0, double e1, double x){
    double t = clamp01((x - e0) / (e1 - e0));
    return t * t * (3 - 2 * t);
}

static inline Cell ramp(const Cell& a, const Cell& b, const Cell& c, double t){
    t = clamp01(t);
    return t < 0.5 ? a.mix(b, t * 2) : b.mix(c, (t - 0.5) * 2);
}

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
            cells.resize(y, vector<Cell>(x, Material().forest));
        }

        void fill(){
            Perlin per = Perlin(x, y);  per.fill(7, 0.5, 6);   // высота
            Perlin mon = Perlin(x, y);  mon.fill(5, 0.5, 8);   // хребты
            Perlin tmp = Perlin(x, y);  tmp.fill(5, 0.5, 4);   // отклонение температуры
            Perlin wet = Perlin(x, y);  wet.fill(5, 0.5, 4);   // влажность

            double sea = per.sea_level(0.35);
            Material m;

            for (int j=0; j<y; j++){
                double pole = fabs(j / (double)y - 0.5) * 2.0;     // 0 на экваторе, 1 у полюса

                // температура по широте: 1 на экваторе, быстро падает к полюсам
                double base_temp = 1.0 - pow(pole, 1.6);

                // сухие пояса вокруг широты ~0.3 (пустыни), влажный экватор
                double dry_band = 0.28 * exp(-pow((pole - 0.3) / 0.14, 2));
                double wet_band = 0.15 * exp(-pow(pole / 0.15, 2));

                for (int i=0; i<x; i++){
                    double h     = per.get_cell(i, j);
                    double ridge = 1.0 - fabs(2.0 * mon.get_cell(i, j) - 1.0);

                    // температура с шумом (границы шапок получаются рваными)
                    double temp = base_temp + (tmp.get_cell(i, j) - 0.5) * 0.35;

                    if (h < sea){
                        // ВОДА: градиент от берега к глубине
                        double depth = clamp01((sea - h) / 0.25);
                        Cell col = ramp(m.shore, m.water, m.deep_water, depth);

                        // морской лёд там, где холодно
                        col = col.mix(m.ice, sstep(0.30, 0.18, temp));
                        cells[j][i] = col;
                    }
                    else {
                        // СУША
                        double land  = h - sea;                    // высота над морем
                        temp -= land * 0.7;                        // в горах холоднее

                        double moist = wet.get_cell(i, j) - dry_band + wet_band;

                        // температура 0.2..1 -> 0..1 для градиента
                        double tt = (temp - 0.2) / 0.8;

                        // сухой и влажный варианты биома, между ними плавный переход
                        Cell dry_col = ramp(m.tundra, m.steppe, m.desert, tt);
                        Cell wet_col = ramp(m.taiga,  m.forest, m.jungle, tt);
                        Cell col = dry_col.mix(wet_col, sstep(0.25, 0.65, moist));

                        // пляж у воды
                        col = col.mix(m.sand, sstep(0.02, 0.006, land));

                        // снег и лёд на холоде
                        col = col.mix(m.snow, sstep(0.28, 0.16, temp));

                        // горные хребты: камень, на самых высоких местах снежные шапки
                        double high = sstep(0.03, 0.08, land);
                        col = col.mix(m.rock, sstep(0.88, 0.95, ridge) * high);
                        col = col.mix(m.snow, sstep(0.95, 0.99, ridge) * high);

                        cells[j][i] = col;
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
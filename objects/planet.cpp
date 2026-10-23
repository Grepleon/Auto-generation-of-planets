#include "draw.cpp"
#include "perlin.cpp"
#include "cell.cpp"
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>

using namespace std;

struct Material {
    // вода: от берега к глубине
    Cell shore      = Cell(30, 110, 125);
    Cell water      = Cell(12, 55, 100);
    Cell deep_water = Cell(4, 20, 50);
    Cell ice        = Cell(225, 235, 240);

    // суша: сухие и влажные биомы, от холодного к жаркому
    Cell tundra     = Cell(120, 115, 95);
    Cell steppe     = Cell(150, 145, 85);
    Cell desert     = Cell(190, 160, 115);
    Cell taiga      = Cell(40, 65, 45);
    Cell forest     = Cell(45, 90, 40);
    Cell jungle     = Cell(25, 70, 35);

    Cell sand       = Cell(200, 185, 140);
    Cell rock       = Cell(105, 98, 92);
    Cell snow       = Cell(240, 242, 245);
};

// 0..1 с ограничением
static inline double clamp01(double v){
    return v < 0 ? 0 : (v > 1 ? 1 : v);
}

// плавная ступенька: 0 при x <= e0, 1 при x >= e1 (можно e0 > e1 - тогда наоборот)
static inline double sstep(double e0, double e1, double x){
    double t = clamp01((x - e0) / (e1 - e0));
    return t * t * (3 - 2 * t);
}

// градиент из трёх цветов: t = 0 -> a, 0.5 -> b, 1 -> c
static inline Cell ramp(const Cell& a, const Cell& b, const Cell& c, double t){
    t = clamp01(t);
    return t < 0.5 ? a.mix(b, t * 2) : b.mix(c, (t - 0.5) * 2);
}

class Planet{
    private:
        int x, y, dx;
        vector<vector<Cell>> cells;
        vector<vector<unsigned char>> water;   // 1 - открытая вода (для блика)
        vector<vector<double>> clouds;         // плотность облаков 0..1
        Material material;
        Drawer& draw;

        int dist(int x1, int y1){
            int dx = x1 - x / 2;
            int dy = y1 - y / 2;
            return sqrt(dx * dx + dy * dy);
        } 

        // билинейная выборка цвета карты; fx, fy - доли карты 0..1
        void sample_color(double fx, double fy, double& r, double& g, double& b){
            double px = fx * x - 0.5, py = fy * y - 0.5;
            int x0 = (int)floor(px), y0 = (int)floor(py);
            double tx = px - x0, ty = py - y0;
            int x1 = (((x0 + 1) % x) + x) % x;
            x0 = ((x0 % x) + x) % x;
            int ya = min(y - 1, max(0, y0)), yb = min(y - 1, max(0, y0 + 1));

            int* c00 = cells[ya][x0].get_color();
            int* c10 = cells[ya][x1].get_color();
            int* c01 = cells[yb][x0].get_color();
            int* c11 = cells[yb][x1].get_color();
            double w00 = (1 - tx) * (1 - ty), w10 = tx * (1 - ty);
            double w01 = (1 - tx) * ty,       w11 = tx * ty;

            r = c00[0] * w00 + c10[0] * w10 + c01[0] * w01 + c11[0] * w11;
            g = c00[1] * w00 + c10[1] * w10 + c01[1] * w01 + c11[1] * w11;
            b = c00[2] * w00 + c10[2] * w10 + c01[2] * w01 + c11[2] * w11;
        }

        double sample_clouds(double fx, double fy){
            double px = fx * x - 0.5, py = fy * y - 0.5;
            int x0 = (int)floor(px), y0 = (int)floor(py);
            double tx = px - x0, ty = py - y0;
            int x1 = (((x0 + 1) % x) + x) % x;
            x0 = ((x0 % x) + x) % x;
            int ya = min(y - 1, max(0, y0)), yb = min(y - 1, max(0, y0 + 1));
            return clouds[ya][x0] * (1 - tx) * (1 - ty) + clouds[ya][x1] * tx * (1 - ty)
                 + clouds[yb][x0] * (1 - tx) * ty       + clouds[yb][x1] * tx * ty;
        }
    
    public:
        Planet(int x, int y, int dx, Drawer& draw): x(x), y(y), dx(dx), draw(draw)
        {
            cells.resize(y, vector<Cell>(x, Material().forest));
            water.resize(y, vector<unsigned char>(x, 0));
            clouds.resize(y, vector<double>(x, 0.0));
        }

        void fill(){
            const double PI = acos(-1.0);

            // весь шум считается на сфере: у полюсов нет растяжения и лучей
            Perlin per = Perlin(x, y);  per.fill_sphere(7, 0.5, 1.4);   // высота
            Perlin mon = Perlin(x, y);  mon.fill_sphere(5, 0.5, 2.2);   // хребты
            Perlin tmp = Perlin(x, y);  tmp.fill_sphere(4, 0.5, 1.0);   // отклонение температуры
            Perlin wet = Perlin(x, y);  wet.fill_sphere(4, 0.5, 1.0);   // влажность
            Perlin det = Perlin(x, y);  det.fill_sphere(2, 0.6, max(4.0, x / 19.0));   // мелкая зернистость
            Perlin cld = Perlin(x, y);  cld.fill_sphere(6, 0.55, 2.0);  // облака

            double sea = per.sea_level(0.35);
            double relief = 0.3;            // сила теней рельефа (больше = контрастнее горы)
            double mount  = 0.18;           // насколько хребты поднимаются над равниной
            Material m;

            // высота с учётом гор: хребты реально приподнимают местность
            auto elev = [&](int i, int j){
                double hh = per.get_cell(i, j);
                double rr = 1.0 - fabs(2.0 * mon.get_cell(i, j) - 1.0);
                double ld = hh - sea;
                if (ld <= 0) return hh;
                return hh + mount * pow(rr, 4) * clamp01(ld / 0.05);
            };

            for (int j=0; j<y; j++){
                double lat  = (0.5 - (j + 0.5) / y) * PI;
                double pole = fabs(lat) / (PI / 2);                // 0 на экваторе, 1 у полюса
                double cosl = max(0.05, cos(lat));
                int ju = max(j - 1, 0), jd = min(j + 1, y - 1);
                double step_y = max(1, jd - ju) * (PI / y);        // шаг по широте в радианах

                // температура по широте: 1 на экваторе, быстро падает к полюсам
                double base_temp = 1.0 - pow(pole, 1.6);

                // сухие пояса вокруг широты ~0.3 (пустыни), влажный экватор
                double dry_band = 0.28 * exp(-pow((pole - 0.3) / 0.14, 2));
                double wet_band = 0.15 * exp(-pow(pole / 0.15, 2));

                for (int i=0; i<x; i++){
                    int il = (i - 1 + x) % x, ir = (i + 1) % x;
                    double step_x = 2.0 * cosl * (2 * PI / x);     // шаг по долготе в радианах

                    double h     = per.get_cell(i, j);
                    double ridge = 1.0 - fabs(2.0 * mon.get_cell(i, j) - 1.0);
                    double d     = det.get_cell(i, j);             // 0..1, мелкий шум

                    // облака
                    clouds[j][i] = sstep(0.50, 0.70, cld.get_cell(i, j)) * (0.85 + 0.15 * d);

                    // температура с шумом (границы шапок получаются рваными)
                    double temp = base_temp + (tmp.get_cell(i, j) - 0.5) * 0.35 + (d - 0.5) * 0.08;

                    if (h < sea){
                        // ВОДА: узкий шельф, потом глубина
                        double depth = clamp01((sea - h) / 0.12);
                        Cell col = ramp(m.shore, m.water, m.deep_water, depth);

                        // морской лёд там, где холодно
                        double ice = sstep(0.27, 0.21, temp);
                        col = col.mix(m.ice, ice);

                        water[j][i] = ice < 0.5 ? 1 : 0;
                        cells[j][i] = col.shade(0.96 + d * 0.08);
                    }
                    else {
                        // СУША
                        water[j][i] = 0;
                        double land  = elev(i, j) - sea;           // высота над морем (с горами)
                        temp -= land * 0.7;                        // в горах холоднее

                        // шум в влажности делает границы биомов рваными, а не размытыми
                        double moist = wet.get_cell(i, j) - dry_band + wet_band + (d - 0.5) * 0.2;

                        // температура 0.2..1 -> 0..1 для градиента
                        double tt = (temp - 0.2) / 0.8;

                        // сухой и влажный варианты биома, между ними узкий переход
                        Cell dry_col = ramp(m.tundra, m.steppe, m.desert, tt);
                        Cell wet_col = ramp(m.taiga,  m.forest, m.jungle, tt);
                        Cell col = dry_col.mix(wet_col, sstep(0.42, 0.52, moist));

                        // пляж у воды (неровная полоса)
                        col = col.mix(m.sand, sstep(0.02, 0.008, land + (d - 0.5) * 0.01));

                        // снег и лёд на холоде
                        col = col.mix(m.snow, sstep(0.25, 0.20, temp));

                        // горы: каменные склоны, снежные шапки только на самых высоких
                        double high = sstep(0.03, 0.07, land);
                        double rock = sstep(0.82, 0.90, ridge) * high;
                        double cap  = sstep(0.90, 0.97, ridge) * sstep(0.10, 0.18, land);
                        col = col.mix(m.rock, rock);
                        col = col.mix(m.snow, cap);

                        // тень рельефа: уклон в радианах, свет с северо-запада
                        double sx = (elev(ir, j) - elev(il, j)) / step_x;
                        double sy = (elev(i, jd) - elev(i, ju)) / step_y;
                        double light = 1.0 + relief * (sx + sy) * (1.0 + 2.0 * rock);
                        light = min(1.3, max(0.7, light));

                        cells[j][i] = col.shade(light * (0.93 + d * 0.14));
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

        // sunx, suny, sunz - направление на солнце (x вправо, y вверх, z к зрителю)
        void show3D(int ddx=0, int ddy=0, double dr=1, double rot = 0.0, double tilt = 0.0,
                    double sunx = -0.7, double suny = 0.3, double sunz = 0.65){
            const double PI = acos(-1.0);

            int size = y * dx;                 // диаметр шара в пикселях
            double R  = size / 2.0 * dr;
            double cx = size / 2.0 + ddx;
            double cy = size / 2.0 + ddy;
            double ct = cos(tilt), st = sin(tilt);

            // нормированное направление света и вектор блика (середина между светом и взглядом)
            double sl = sqrt(sunx * sunx + suny * suny + sunz * sunz);
            double lx = sunx / sl, ly = suny / sl, lz = sunz / sl;
            double hx = lx, hy = ly, hz = lz + 1.0;
            double hl = sqrt(hx * hx + hy * hy + hz * hz);
            hx /= hl; hy /= hl; hz /= hl;

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

                    // положение на карте (доли 0..1)
                    double fx = fmod(lon / (2 * PI) + 0.5, 1.0);
                    if (fx < 0) fx += 1.0;
                    double fy = clamp01(0.5 - lat / PI);

                    // цвет поверхности (плавная выборка)
                    double r, g, b;
                    sample_color(fx, fy, r, g, b);

                    int ix = min(x - 1, (int)(fx * x));
                    int iy = min(y - 1, (int)(fy * y));
                    bool is_water = water[iy][ix] != 0;

                    // облака двигаются чуть быстрее поверхности
                    double cfx = fmod(fx + rot * 0.12 / (2 * PI), 1.0);
                    if (cfx < 0) cfx += 1.0;
                    double cl = sample_clouds(cfx, fy) * 0.9;
                    r = r * (1 - cl) + 245 * cl;
                    g = g * (1 - cl) + 247 * cl;
                    b = b * (1 - cl) + 250 * cl;

                    // освещение: нормаль точки на шаре - это (u, v, w)
                    double diff = u * lx + v * ly + w * lz;
                    double lam  = clamp01(diff);
                    double light = 0.04 + 0.96 * pow(lam, 0.75);   // 0.04 - ночной ambient

                    r *= light; g *= light; b *= light;

                    // блик на воде (не на облаках)
                    if (is_water && diff > 0){
                        double nh = max(0.0, u * hx + v * hy + w * hz);
                        double spec = pow(nh, 90) * 0.6 * (1 - cl);
                        r += 255 * spec; g += 255 * spec; b += 255 * spec;
                    }

                    // атмосфера: голубое свечение у края диска, сильнее на дневной стороне
                    double rim = pow(1.0 - w, 3.0) * 0.8 * clamp01(diff + 0.25);
                    r += 70 * rim; g += 130 * rim; b += 255 * rim;

                    Cell out((int)min(255.0, r), (int)min(255.0, g), (int)min(255.0, b));
                    draw.rectangle(px, py, px + dx, py + dx, out.get_color());
                }
            }
        }
};
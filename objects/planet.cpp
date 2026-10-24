#pragma once
#include "draw.cpp"
#include "perlin.cpp"
#include "cell.cpp"
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>

using namespace std;

// ============================================================
//  Математика
// ============================================================

struct Vec3 {
    double x = 0, y = 0, z = 0;
    Vec3() {}
    Vec3(double x, double y, double z): x(x), y(y), z(z) {}

    Vec3 operator+(const Vec3& o) const { return Vec3(x + o.x, y + o.y, z + o.z); }
    Vec3 operator-(const Vec3& o) const { return Vec3(x - o.x, y - o.y, z - o.z); }
    Vec3 operator*(double k)      const { return Vec3(x * k, y * k, z * k); }
    double dot(const Vec3& o)     const { return x * o.x + y * o.y + z * o.z; }
    double len()                  const { return sqrt(x * x + y * y + z * z); }
    Vec3 norm() const {
        double l = len();
        return l > 1e-12 ? Vec3(x / l, y / l, z / l) : Vec3(0, 0, 1);
    }
};

// Камера смотрит вдоль +Z; X вправо, Y вверх. Перспективная проекция.
struct Camera {
    Vec3 pos;
    int w, h;          // размер холста в пикселях
    double focal;      // фокусное расстояние в пикселях

    Camera(int w, int h, Vec3 pos = Vec3(0, 0, -10), double fov_deg = 40.0)
        : pos(pos), w(w), h(h)
    {
        const double PI = acos(-1.0);
        focal = (h / 2.0) / tan(fov_deg * PI / 360.0);
    }

    // единичный луч через пиксель (px, py)
    Vec3 ray(double px, double py) const {
        return Vec3((px - w / 2.0) / focal, -(py - h / 2.0) / focal, 1.0).norm();
    }
};

class Planet;

// Точечный источник света (например, Солнце)
struct Light {
    Vec3 pos;
    double r = 1, g = 1, b = 1;     // цвет 0..1
    double intensity = 1.0;
    double range = 0.0;             // 0 = без затухания; иначе яркость ~ 1 / (1 + (d/range)^2)
    const Planet* owner = nullptr;  // тело, которое светит (оно не отбрасывает тень на свет)
};

// Всё, что нужно телу для отрисовки: камера, источники света и другие тела (для теней)
struct RenderContext {
    Camera cam;
    vector<Light> lights;
    vector<const Planet*> bodies;
    RenderContext(const Camera& c): cam(c) {}
};

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

// ============================================================
//  Планета
// ============================================================

class Planet{
    protected:
        int x, y, dx;                          // x, y - размер карты поверхности; dx - размер блока пикселей при выводе
        vector<vector<Cell>> cells;
        vector<vector<unsigned char>> water;   // 1 - открытая вода (для блика)
        vector<vector<double>> clouds;         // плотность облаков 0..1
        Material material;
        Drawer& draw;

        Vec3 pos;            // положение центра в мировых координатах
        double radius;       // радиус в мировых единицах
        double rot  = 0.0;   // поворот вокруг своей оси (радианы)
        double tilt = 0.0;   // наклон оси (радианы)

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

        // Нормаль в мировых координатах -> широта (sin) и "базовая" долгота с учётом наклона оси.
        // Ориентация тела не зависит от камеры.
        void local_coords(const Vec3& N, double& sin_lat, double& lon0) const {
            double ct = cos(tilt), st = sin(tilt);
            double w  = -N.z;
            sin_lat   = max(-1.0, min(1.0, N.y * ct - w * st));
            double w2 = N.y * st + w * ct;
            lon0      = atan2(N.x, w2);
        }

        // долгота/широта -> доли карты 0..1
        void to_map(double lon, double lat, double& fx, double& fy) const {
            const double PI = acos(-1.0);
            fx = fmod(lon / (2 * PI) + 0.5, 1.0);
            if (fx < 0) fx += 1.0;
            fy = clamp01(0.5 - lat / PI);
        }

        // Доля света, дошедшего до точки P от источника L (0 = полная тень, 1 = свет).
        // Тени мягкие: учитывается угловой размер и источника, и затмевающего тела.
        double visibility(const Vec3& P, const Vec3& l, double dl, const Light& L,
                          const RenderContext& ctx) const {
            double vis = 1.0;
            double rl  = L.owner ? L.owner->radius : 0.0;
            double al  = asin(min(1.0, rl / dl));            // угловой радиус источника

            for (const Planet* b : ctx.bodies){
                if (b == this || b == L.owner) continue;
                Vec3 o = b->pos - P;
                double d = o.len();
                if (d <= b->radius) continue;
                double along = o.dot(l);
                if (along <= 0 || along >= dl) continue;     // тело не между точкой и светом

                double sep = acos(max(-1.0, min(1.0, along / d)));   // угол между светом и телом
                double ao  = asin(b->radius / d);                    // угловой радиус тела
                double e0  = ao - al, e1 = ao + al;
                if (e1 - e0 < 1e-4) e1 = e0 + 1e-4;
                vis = min(vis, sstep(e0, e1, sep));
            }
            return vis;
        }

        // Цвет видимой точки поверхности.
        // P - точка в мире, N - нормаль, V - единичный вектор от точки к камере.
        virtual Cell shade_point(const Vec3& P, const Vec3& N, const Vec3& V,
                                 const RenderContext& ctx){
            const double PI = acos(-1.0);

            double sin_lat, lon0;
            local_coords(N, sin_lat, lon0);
            double lat = asin(sin_lat);
            double fx, fy;
            to_map(lon0 + rot, lat, fx, fy);

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

            // освещение от всех источников
            double lr = 0, lg = 0, lb = 0;       // диффузный свет
            double sr = 0, sg = 0, sb = 0;       // блик
            double best = -1.0;                  // наибольший косинус угла к свету (для атмосферы)

            for (const Light& L : ctx.lights){
                if (L.owner == this) continue;

                Vec3 tl = L.pos - P;
                double dl = tl.len();
                if (dl < 1e-9) continue;
                Vec3 l = tl * (1.0 / dl);

                double diff  = N.dot(l);
                double atten = L.range > 0 ? 1.0 / (1.0 + (dl / L.range) * (dl / L.range)) : 1.0;
                best = max(best, diff * atten);
                if (diff <= 0) continue;

                double k = L.intensity * atten * visibility(P, l, dl, L, ctx);
                if (k <= 0) continue;

                double lam = pow(diff, 0.75) * k;
                lr += L.r * lam;  lg += L.g * lam;  lb += L.b * lam;

                if (is_water){
                    Vec3 H = (l + V).norm();
                    double nh   = max(0.0, N.dot(H));
                    double spec = pow(nh, 90) * 0.35 * (1 - cl) * k;
                    sr += L.r * spec;  sg += L.g * spec;  sb += L.b * spec;
                }
            }

            const double amb = 0.04;             // ночной ambient
            r *= amb + 0.96 * min(1.5, lr);
            g *= amb + 0.96 * min(1.5, lg);
            b *= amb + 0.96 * min(1.5, lb);

            r += 255 * sr;  g += 255 * sg;  b += 255 * sb;

            // атмосфера: голубое свечение у края диска, сильнее на дневной стороне
            double facing = clamp01(N.dot(V));
            double rim = pow(1.0 - facing, 3.0) * 0.8 * clamp01(best + 0.25);
            r += 70 * rim;  g += 130 * rim;  b += 255 * rim;

            return Cell((int)min(255.0, r), (int)min(255.0, g), (int)min(255.0, b));
        }

        // Экранный прямоугольник, который может занимать тело. false - тело не видно.
        bool screen_box(const Camera& cam, int& x0, int& y0, int& x1, int& y1) const {
            const double PI = acos(-1.0);
            Vec3 c = pos - cam.pos;
            double d = c.len();
            if (c.z <= 0 || d <= radius) return false;

            double a     = asin(radius / d);            // угловой радиус
            double theta = acos(max(-1.0, min(1.0, c.z / d)));   // угол от оси камеры
            double cxp = cam.w / 2.0 + cam.focal * c.x / c.z;
            double cyp = cam.h / 2.0 - cam.focal * c.y / c.z;

            double rpx;
            if (theta + a >= PI / 2 - 0.02) rpx = 1e7;
            else rpx = cam.focal * max(tan(theta + a) - tan(theta), tan(theta) - tan(theta - a)) * 1.1 + 2;

            x0 = (int)max(0.0, floor(cxp - rpx));
            y0 = (int)max(0.0, floor(cyp - rpx));
            x1 = (int)min((double)cam.w, ceil(cxp + rpx));
            y1 = (int)min((double)cam.h, ceil(cyp + rpx));
            if (x0 >= x1 || y0 >= y1) return false;

            x0 -= x0 % dx;
            y0 -= y0 % dx;
            return true;
        }

    public:
        Planet(int x, int y, int dx, Drawer& draw, Vec3 pos = Vec3(0, 0, 0), double radius = 1.0)
            : x(x), y(y), dx(dx), draw(draw), pos(pos), radius(radius)
        {
            cells.resize(y, vector<Cell>(x, Material().forest));
            water.resize(y, vector<unsigned char>(x, 0));
            clouds.resize(y, vector<double>(x, 0.0));
        }

        virtual ~Planet(){}

        // ---------- положение и ориентация ----------
        void   set_pos(const Vec3& p)            { pos = p; }
        void   set_radius(double r)              { radius = r; }
        void   set_pose(double rot_, double tilt_) { rot = rot_; tilt = tilt_; }
        Vec3   get_pos()    const { return pos; }
        double get_radius() const { return radius; }

        // Если тело светит само, заполняет out и возвращает true
        virtual bool emit_light(Light& out) const { return false; }

        // Рисуется до всех тел (свечение, корона)
        virtual void draw_back(const RenderContext& ctx){}

        // ---------- генерация поверхности ----------
        virtual void fill(){
            const double PI = acos(-1.0);

            // весь шум считается на сфере: у полюсов нет растяжения и лучей
            Perlin per = Perlin(x, y);  per.fill_sphere(7, 0.5, 1.4);   // высота
            Perlin mon = Perlin(x, y);  mon.fill_sphere(5, 0.5, 2.2);   // хребты
            Perlin tmp = Perlin(x, y);  tmp.fill_sphere(4, 0.5, 1.0);   // отклонение температуры
            Perlin wet = Perlin(x, y);  wet.fill_sphere(4, 0.5, 1.0);   // влажность
            Perlin det = Perlin(x, y);  det.fill_sphere(2, 0.6, max(4.0, x / 19.0));   // мелкая зернистость
            Perlin cld = Perlin(x, y);  cld.fill_sphere(6, 0.55, 2.0);  // облака

            double sea = per.sea_level(0.35);
            // Тени рельефа теперь "запечены" в текстуру, а свет движется, поэтому они слабые:
            // нужны лишь как намёк на объём. Основное освещение считается при выводе.
            double relief = 0.15;
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

                        // лёгкая тень рельефа (см. комментарий к relief)
                        double sx = (elev(ir, j) - elev(il, j)) / step_x;
                        double sy = (elev(i, jd) - elev(i, ju)) / step_y;
                        double light = 1.0 + relief * (sx + sy) * (1.0 + 2.0 * rock);
                        light = min(1.2, max(0.8, light));

                        cells[j][i] = col.shade(light * (0.93 + d * 0.14));
                    }
                }
            }
        }

        // плоская карта поверхности
        virtual void show(){
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

        // Объёмный вывод: трассировка луча от камеры через каждый пиксель до сферы.
        // Общий для всех тел; различается только shade_point().
        virtual void show3D(const RenderContext& ctx){
            const Camera& cam = ctx.cam;

            int x0, y0, x1, y1;
            if (!screen_box(cam, x0, y0, x1, y1)) return;

            Vec3 oc = cam.pos - pos;
            double cc = oc.dot(oc) - radius * radius;

            for (int py = y0; py < y1; py += dx){
                for (int px = x0; px < x1; px += dx){
                    Vec3 D = cam.ray(px + dx / 2.0, py + dx / 2.0);

                    // пересечение луча со сферой
                    double bb   = oc.dot(D);
                    double disc = bb * bb - cc;
                    if (disc < 0) continue;
                    double t = -bb - sqrt(disc);
                    if (t <= 0) continue;

                    Vec3 P = cam.pos + D * t;
                    Vec3 N = (P - pos) * (1.0 / radius);
                    Vec3 V = D * -1.0;

                    Cell out = shade_point(P, N, V, ctx);
                    draw.rectangle(px, py, px + dx, py + dx, out.get_color());
                }
            }
        }
};
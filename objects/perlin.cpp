#pragma once
#include <vector>
#include <random>
#include <algorithm>
#include <cmath>

using namespace std;

// один генератор на всю программу
inline std::mt19937& get_gen(){
    static std::mt19937 gen{std::random_device{}()};
    return gen;
}

inline int get_rangom(){
    std::uniform_int_distribution<int> distrib(1, 100);
    return distrib(get_gen());
}

inline double get_rangom_double(){
    std::uniform_real_distribution<double> distrib(0.0, 1.0);
    return distrib(get_gen());
}

// ---------- 3D шум (для сферы) ----------

// псевдослучайное число 0..1 для узла целочисленной решётки
inline double hash3(int ix, int iy, int iz, unsigned seed){
    unsigned h = (unsigned)ix * 374761393u + (unsigned)iy * 668265263u
               + (unsigned)iz * 2147483629u + seed * 1442695041u;
    h = (h ^ (h >> 13)) * 1274126177u;
    h ^= h >> 16;
    return (h & 0xFFFFFFu) / 16777216.0;
}

// гладкий шум: трилинейная интерполяция случайных значений в узлах
inline double noise3(double x, double y, double z, unsigned seed){
    double fx = floor(x), fy = floor(y), fz = floor(z);
    int ix = (int)fx, iy = (int)fy, iz = (int)fz;
    double tx = x - fx, ty = y - fy, tz = z - fz;
    tx = tx * tx * (3 - 2 * tx);
    ty = ty * ty * (3 - 2 * ty);
    tz = tz * tz * (3 - 2 * tz);

    auto h = [&](int a, int b, int c){ return hash3(ix + a, iy + b, iz + c, seed); };

    double x00 = h(0,0,0) * (1 - tx) + h(1,0,0) * tx;
    double x10 = h(0,1,0) * (1 - tx) + h(1,1,0) * tx;
    double x01 = h(0,0,1) * (1 - tx) + h(1,0,1) * tx;
    double x11 = h(0,1,1) * (1 - tx) + h(1,1,1) * tx;
    double y0 = x00 * (1 - ty) + x10 * ty;
    double y1 = x01 * (1 - ty) + x11 * ty;
    return y0 * (1 - tz) + y1 * tz;
}

// сумма октав; между октавами координаты поворачиваются и удваиваются,
// чтобы не было заметных осей решётки
inline double fbm3(double x, double y, double z, int octaves, double pers, unsigned seed){
    double sum = 0, amp = 1, tot = 0;
    for (int k = 0; k < octaves; k++){
        sum += amp * noise3(x, y, z, seed + 101u * k);
        tot += amp;
        amp *= pers;
        double nx =  0.00 * x + 1.60 * y + 1.20 * z + 3.7;
        double ny = -1.60 * x + 0.72 * y - 0.96 * z + 1.3;
        double nz = -1.20 * x - 0.96 * y + 1.28 * z + 5.1;
        x = nx; y = ny; z = nz;
    }
    return sum / tot;
}

class Perlin{
private:
    vector<vector<double>> a;
    int x;
    int y;

    // одна октава: грубая сетка gx*gy со случайными значениями,
    // растягивается на всю карту сглаженной интерполяцией
    void add_octave(int gx, int gy, double amp){
        vector<vector<double>> g(gy, vector<double>(gx));
        for (int i=0; i<gy; i++)
            for (int j=0; j<gx; j++)
                g[i][j] = get_rangom_double();

        auto fade = [](double t){ return t * t * (3 - 2 * t); };

        for (int i=0; i<y; i++){
            double fy = (double)i * gy / y;
            int y0 = (int)fy;
            int y1 = (y0 + 1) % gy;          // замыкание по тору
            double ty = fade(fy - y0);

            for (int j=0; j<x; j++){
                double fx = (double)j * gx / x;
                int x0 = (int)fx;
                int x1 = (x0 + 1) % gx;
                double tx = fade(fx - x0);

                double top = g[y0][x0] * (1 - tx) + g[y0][x1] * tx;
                double bot = g[y1][x0] * (1 - tx) + g[y1][x1] * tx;
                a[i][j] += amp * (top * (1 - ty) + bot * ty);
            }
        }
    }

    void normalize(){
        double mn = 1e18, mx = -1e18;
        for (auto& row : a) for (double v : row) { mn = min(mn, v); mx = max(mx, v); }
        if (mx <= mn) return;
        for (auto& row : a) for (double& v : row) v = (v - mn) / (mx - mn);
    }

public:
    Perlin(int x, int y){
        this->x = x;
        this->y = y;
        a.resize(y, vector<double>(x, 0));
    }

    // плоский шум на карте (как раньше)
    // octaves - число слоёв детализации
    // persistence - во сколько раз падает вклад каждой следующей октавы (0.4..0.55)
    // base - размер сетки самой крупной октавы (меньше = крупнее континенты)
    void fill(int octaves = 6, double persistence = 0.5, int base = 2){
        for (auto& row : a)
            std::fill(row.begin(), row.end(), 0.0);

        double amp = 1.0;
        for (int k=0; k<octaves; k++){
            int gx = base << k;
            if (gx > x) break;                       // мельче пикселя смысла нет
            int gy = max(1, (int)((double)gx * y / x + 0.5));  // ячейки остаются квадратными
            add_octave(gx, gy, amp);
            amp *= persistence;
        }
        normalize();
    }

    // шум на СФЕРЕ: каждая клетка карты (долгота, широта) переводится в точку
    // на единичной сфере, и шум считается в 3D. Нет растяжения у полюсов и нет шва.
    // freq - число "ячеек" шума на единичный радиус: 1.0..1.5 даёт крупные континенты,
    // больше - мельче формы
    void fill_sphere(int octaves = 6, double persistence = 0.5, double freq = 1.5){
        const double PI = 3.14159265358979323846;
        unsigned seed = (unsigned)get_gen()();

        vector<double> sl(x), cl(x);
        for (int i=0; i<x; i++){
            double lon = ((i + 0.5) / x - 0.5) * 2 * PI;
            sl[i] = sin(lon);
            cl[i] = cos(lon);
        }

        for (int j=0; j<y; j++){
            double lat = (0.5 - (j + 0.5) / y) * PI;
            double cy = cos(lat), sy = sin(lat);
            for (int i=0; i<x; i++){
                a[j][i] = fbm3(cy * sl[i] * freq, sy * freq, cy * cl[i] * freq,
                               octaves, persistence, seed);
            }
        }
        normalize();
    }

    // порог высоты, при котором доля суши равна land_fraction (например 0.3)
    double sea_level(double land_fraction){
        vector<double> t;
        t.reserve((size_t)x * y);
        for (auto& row : a) t.insert(t.end(), row.begin(), row.end());
        size_t k = (size_t)((1.0 - land_fraction) * (t.size() - 1));
        std::nth_element(t.begin(), t.begin() + k, t.end());
        return t[k];
    }

    double get_cell(int x, int y){
        return a[y][x];
    }
};
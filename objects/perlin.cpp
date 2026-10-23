#pragma once
#include <vector>
#include <random>
#include <algorithm>

using namespace std;

// один генератор на всю программу (раньше random_device создавался при каждом вызове - это медленно)
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

public:
    Perlin(int x, int y){
        this->x = x;
        this->y = y;
        a.resize(y, vector<double>(x, 0));
    }

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
            int gy = max(1, (int)((long long)gx * y / x));  // ячейки остаются квадратными
            add_octave(gx, gy, amp);
            amp *= persistence;
        }

        double mn = 1e18, mx = -1e18;
        for (auto& row : a) for (double v : row) { mn = min(mn, v); mx = max(mx, v); }
        if (mx <= mn) return;
        for (auto& row : a) for (double& v : row) v = (v - mn) / (mx - mn);
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
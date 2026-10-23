#pragma once
#include <vector>
#include <random>

using namespace std;


int get_rangom(){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> distrib(1, 100);
    return distrib(gen);
}

class Perlin{
private:
    vector<vector<double>> a;
    int x;
    int y;

public:
    Perlin(int x, int y){
        this->x = x;
        this->y = y;
        a.resize(y, vector<double>(x, 0));
        for (int i=0; i<y; i++){
            for (int j=0; j<x; j++){
                if (get_rangom() >= 95) a[i][j] = 1;
            }
        }
    }

    void fill(){
        for (int k=0; k<7; k++){
            vector<vector<double>> b(y, vector<double>(x));
            for (int i=0; i<y; i++){
                for (int j=0; j<x; j++){
                    b[i][j] = (
                        a[i][j] + 
                        a[(i + 1) % y][j] + 
                        a[(i - 1 + y) % y][j] + 
                        a[i][(j + 1) % x] + 
                        a[i][(j - 1 + x) % x]
                    ) / 4.5;
                }
            }
            swap(a, b);
        }
         double mn = 1e18, mx = -1e18;
        for (auto& row : a) for (double v : row) { mn = min(mn, v); mx = max(mx, v); }
        for (auto& row : a) for (double& v : row) v = (v - mn) / (mx - mn);
    }

    double get_cell(int x, int y){
        return a[y][x];
    }
};
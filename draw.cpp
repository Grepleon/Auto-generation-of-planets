#include "stb_image_write.h"
#include "draw.h"
#include <vector>
#include <string>
#include <cmath>
#pragma once

class Drawer:Draw{
    private:
        int width;
        int height;
        const int channels = 3; // RGB
        std::vector<unsigned char> img;
        void setBlack(int i, int j) {
            if (i < 0 || i >= height || j < 0 || j >= width) return;
            unsigned char* p = &img[(i * width + j) * channels];
            p[0] = p[1] = p[2] = 0;
        };
        void setWhite(int i, int j) {
            if (i < 0 || i >= height || j < 0 || j >= width) return;
            unsigned char* p = &img[(i * width + j) * channels];
            p[0] = p[1] = p[2] = 255;
        };
        void setColor(int i, int j, int* color) {
            if (i < 0 || i >= height || j < 0 || j >= width) return;
            unsigned char* p = &img[(i * width + j) * channels];
            p[0] = color[0];
            p[1] = color[1];
            p[2] = color[2];

        };

    public:
        Drawer(int w, int h){
            width = w;
            height = h;
            img.resize(width * height * channels, 255);
        }

        Drawer(){}

        void blackRectangle(int x1, int y1, int x2, int y2){
            for (int y = y1; y <= y2; ++y)
                for (int x = x1; x <= x2; ++x)
                    setBlack(y, x);
        }

        void rectangle(int x1, int y1, int x2, int y2, int* color){
            for (int y = y1; y <= y2; ++y)
                for (int x = x1; x <= x2; ++x)
                    setColor(y, x, color);
        }

        void circle(int x, int y, int r, int* color){
            int y1 = y - r;
            int y2 = y + r;
            int x1 = x - r;
            int x2 = x + r;
            for (int _y = y1; _y <= y2; ++_y)
                for (int _x = x1; _x <= x2; ++_x)
                    if (sqrt((_x - x) * (_x - x) + (_y - y) * (_y - y)) <= r) setColor(_y, _x, color);
        }

        void oval(int x1, int y1, int x2, int y2, int* color){
            int x = (x1 + x2) / 2;
            int y = (y1 + y2) / 2;
            int r = (x - x1 > y - y1)?y-y1:x-x1;
            double dx = (x - x1) / 1.0 / r;
            double dy = (y - y1) / 1.0 / r;
            for (int _y = y1; _y <= y2; ++_y)
                for (int _x = x1; _x <= x2; ++_x)
                    if (sqrt((_x - x) * (_x - x) / dx / dx + (_y - y) * (_y - y) / dy / dy) <= r) setColor(_y, _x, color);
        }

        void Save(std::string path_save){
            stbi_write_png(path_save.c_str(), width, height, channels, img.data(), width * channels);
        }
};
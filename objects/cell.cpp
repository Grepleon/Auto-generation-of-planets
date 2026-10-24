#pragma once

class Cell {
    private:
        int c[3];

        static int clamp255(double v){
            return v < 0 ? 0 : (v > 255 ? 255 : (int)(v + 0.5));
        }
    public:
        Cell(int r, int g, int b) : c{r, g, b} {}
        int* get_color() { return c; }

        // плавное смешивание: t = 0 даёт этот цвет, t = 1 даёт цвет o
        Cell mix(const Cell& o, double t) const {
            if (t < 0) t = 0;
            if (t > 1) t = 1;
            return Cell(
                (int)(c[0] + (o.c[0] - c[0]) * t + 0.5),
                (int)(c[1] + (o.c[1] - c[1]) * t + 0.5),
                (int)(c[2] + (o.c[2] - c[2]) * t + 0.5));
        }

        // яркость: f < 1 темнее, f > 1 светлее
        Cell shade(double f) const {
            return Cell(clamp255(c[0] * f), clamp255(c[1] * f), clamp255(c[2] * f));
        }
};
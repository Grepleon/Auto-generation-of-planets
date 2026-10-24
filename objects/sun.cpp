#pragma once
#include "planet.cpp"

struct SunMaterial {
    Cell cool   = Cell(150, 35, 0);      // тёмные края гранул
    Cell mid    = Cell(255, 120, 10);
    Cell warm   = Cell(255, 200, 70);
    Cell hot    = Cell(255, 245, 200);   // яркие факелы
    Cell umbra  = Cell(40, 8, 0);        // ядро пятна
    Cell penum  = Cell(120, 40, 5);      // полутень пятна
};

class Sun : public Planet {
    private:
        SunMaterial sm;
        int canvas_w, canvas_h;   // 0 = без ограничения
        double corona;            // размер короны в радиусах диска

    public:
        Sun(int x, int y, int dx, Drawer& draw, int canvas_w = 0, int canvas_h = 0, double corona = 0.8)
            : Planet(x, y, dx, draw), canvas_w(canvas_w), canvas_h(canvas_h), corona(corona) {}

        // Поверхность: грануляция + пятна + факелы. Воды и облаков нет.
        void fill() override {
            const double PI = acos(-1.0);

            Perlin gran  = Perlin(x, y); gran.fill_sphere(6, 0.55, max(8.0, x / 8.0));  // мелкие гранулы
            Perlin meso  = Perlin(x, y); meso.fill_sphere(4, 0.5, 3.0);                 // крупные ячейки
            Perlin spots = Perlin(x, y); spots.fill_sphere(3, 0.5, 2.0);                // пятна
            Perlin fac   = Perlin(x, y); fac.fill_sphere(4, 0.5, 4.0);                  // факелы

            for (int j = 0; j < y; j++){
                double lat  = (0.5 - (j + 0.5) / y) * PI;
                double pole = fabs(lat) / (PI / 2);
                // пятна живут в "королевских зонах" около ±30° и почти не встречаются у экватора и полюсов
                double zone = exp(-pow((pole - 0.35) / 0.22, 2));

                for (int i = 0; i < x; i++){
                    double g = gran.get_cell(i, j);
                    double m = meso.get_cell(i, j);
                    double f = fac.get_cell(i, j);

                    // яркость фотосферы: гранулы поверх крупных ячеек
                    double t = clamp01(0.55 * g + 0.45 * m);
                    t = sstep(0.25, 0.80, t);
                    Cell col = ramp(sm.cool, sm.mid, sm.warm, t);

                    // факелы: светлые пятна рядом с активными зонами
                    double faculae = sstep(0.62, 0.80, f) * (0.3 + 0.7 * zone);
                    col = col.mix(sm.hot, faculae * 0.8);

                    // солнечные пятна: полутень шире и светлее, ядро темнее
                    double s = spots.get_cell(i, j) - 0.2 * (1.0 - zone);
                    double pen = sstep(0.62, 0.68, s);
                    double umb = sstep(0.69, 0.74, s);
                    col = col.mix(sm.penum, pen * 0.9);
                    col = col.mix(sm.umbra, umb);

                    cells[j][i] = col;
                    water[j][i] = 0;
                    clouds[j][i] = 0;
                }
            }
        }

        // Солнце светит само: направление света sunx/suny/sunz игнорируется.
        void show3D(int ddx=0, int ddy=0, double dr=1, double rot = 0.0, double tilt = 0.0,
                    double sunx = -0.7, double suny = 0.3, double sunz = 0.65) override {
            const double PI = acos(-1.0);

            int size = y * dx;
            double R  = size / 2.0 * dr;
            double cx = size / 2.0 + ddx;
            double cy = size / 2.0 + ddy;
            double ct = cos(tilt), st = sin(tilt);

            double reach = R * (1.0 + corona);
            int x0 = max(0, (int)floor((cx - reach) / dx) * dx);
            int y0 = max(0, (int)floor((cy - reach) / dx) * dx);
            int x1 = (int)(cx + reach);
            int y1 = (int)(cy + reach);

            for (int py = y0; py < y1; py += dx){
                if (canvas_h > 0 && py >= canvas_h) break;
                for (int px = x0; px < x1; px += dx){
                    if (canvas_w > 0 && px >= canvas_w) break;

                    double u =  (px + dx / 2.0 - cx) / R;
                    double v = -(py + dx / 2.0 - cy) / R;
                    double r2 = u * u + v * v;

                    // ---------- корона (за пределами диска) ----------
                    if (r2 > 1.0){
                        double d = sqrt(r2) - 1.0;                       // расстояние от края в радиусах
                        double a = atan2(v, u);
                        // лучи короны: медленно меняются с вращением
                        double streak = 0.8 + 0.2 * sin(5 * a + rot * 0.5) * sin(3 * a - rot * 0.3);
                        double glow = (exp(-d * 6.0) * 0.85 + exp(-d * 1.8) * 0.18) * streak;
                        if (glow < 0.01) continue;
                        glow = min(1.0, glow);
                        Cell out((int)(255 * glow), (int)(190 * glow), (int)(110 * glow * glow));
                        draw.rectangle(px, py, px + dx, py + dx, out.get_color());
                        continue;
                    }

                    // ---------- диск ----------
                    double w = sqrt(1.0 - r2);                           // 1 в центре, 0 на краю

                    double v2 = v * ct - w * st;
                    double w2 = v * st + w * ct;
                    double lat = asin(max(-1.0, min(1.0, v2)));

                    // дифференциальное вращение: экватор крутится быстрее полюсов
                    double lon = atan2(u, w2) + rot * (1.0 - 0.15 * v2 * v2);

                    double fx = fmod(lon / (2 * PI) + 0.5, 1.0);
                    if (fx < 0) fx += 1.0;
                    double fy = clamp01(0.5 - lat / PI);

                    double r, g, b;
                    sample_color(fx, fy, r, g, b);

                    // потемнение к краю: края темнее и краснее
                    double k = 1.0 - 0.6 * (1.0 - w);
                    r *= k;
                    g *= pow(k, 1.3);
                    b *= pow(k, 2.0);

                    // лёгкое свечение (bloom) у самого края
                    double rim = pow(1.0 - w, 6.0) * 0.25;
                    r += 255 * rim; g += 150 * rim; b += 40 * rim;

                    Cell out((int)min(255.0, r), (int)min(255.0, g), (int)min(255.0, b));
                    draw.rectangle(px, py, px + dx, py + dx, out.get_color());
                }
            }
        }
};
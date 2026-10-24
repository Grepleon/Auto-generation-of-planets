#pragma once
#include "planet.cpp"

struct SunMaterial {
    Cell cool   = Cell(190, 60, 5);      // тёмные края гранул
    Cell mid    = Cell(255, 140, 25);
    Cell warm   = Cell(255, 205, 90);
    Cell hot    = Cell(255, 240, 180);   // яркие участки
    Cell penum  = Cell(150, 60, 15);     // полутень пятна
    Cell umbra  = Cell(95, 35, 8);       // ядро пятна (не чёрное)
};

class Sun : public Planet {
    private:
        SunMaterial sm;
        double corona;        // размер короны в радиусах диска
        double spot_amount;   // 0 = без пятен, 1 = много (но всё равно небольших)
        double intensity;     // яркость света, который Солнце даёт другим телам

    protected:
        // Солнце светит само: освещение не считается, только гранулы + потемнение к краю
        Cell shade_point(const Vec3& P, const Vec3& N, const Vec3& V,
                         const RenderContext& ctx) override {
            double sin_lat, lon0;
            local_coords(N, sin_lat, lon0);
            double lat = asin(sin_lat);

            // дифференциальное вращение: экватор крутится быстрее полюсов
            double lon = lon0 + rot * (1.0 - 0.15 * sin_lat * sin_lat);

            double fx, fy;
            to_map(lon, lat, fx, fy);

            double r, g, b;
            sample_color(fx, fy, r, g, b);

            // потемнение к краю: края темнее и краснее
            double w = clamp01(N.dot(V));          // 1 в центре, 0 на краю
            double k = 1.0 - 0.55 * (1.0 - w);
            r *= k;
            g *= pow(k, 1.2);
            b *= pow(k, 1.8);

            // лёгкое свечение у самого края
            double rim = pow(1.0 - w, 6.0) * 0.2;
            r += 255 * rim;  g += 150 * rim;  b += 40 * rim;

            return Cell((int)min(255.0, r), (int)min(255.0, g), (int)min(255.0, b));
        }

    public:
        Sun(int x, int y, int dx, Drawer& draw, Vec3 pos = Vec3(0, 0, 0), double radius = 1.0,
            double corona = 0.6, double spot_amount = 0.3, double intensity = 1.0)
            : Planet(x, y, dx, draw, pos, radius),
              corona(corona), spot_amount(spot_amount), intensity(intensity) {}

        // Солнце - источник света сцены
        bool emit_light(Light& out) const override {
            out.pos = pos;
            out.r = 1.0;  out.g = 0.95;  out.b = 0.85;
            out.intensity = intensity;
            out.range = 0.0;
            out.owner = this;
            return true;
        }

        // Поверхность: грануляция, слабые факелы и небольшие мягкие пятна
        void fill() override {
            const double PI = acos(-1.0);

            Perlin gran  = Perlin(x, y); gran.fill_sphere(6, 0.55, max(8.0, x / 8.0));  // мелкие гранулы
            Perlin meso  = Perlin(x, y); meso.fill_sphere(4, 0.5, 3.0);                 // крупные ячейки
            Perlin spots = Perlin(x, y); spots.fill_sphere(3, 0.5, 7.0);                // пятна (мелкие)
            Perlin fac   = Perlin(x, y); fac.fill_sphere(4, 0.5, 5.0);                  // факелы

            // порог появления пятен: чем больше spot_amount, тем ниже порог
            double thr = 0.80 - 0.10 * clamp01(spot_amount);

            for (int j = 0; j < y; j++){
                double lat  = (0.5 - (j + 0.5) / y) * PI;
                double pole = fabs(lat) / (PI / 2);
                // пятна живут в зонах около ±30° и почти не встречаются у экватора и полюсов
                double zone = exp(-pow((pole - 0.35) / 0.22, 2));

                for (int i = 0; i < x; i++){
                    double g = gran.get_cell(i, j);
                    double m = meso.get_cell(i, j);
                    double f = fac.get_cell(i, j);

                    // яркость фотосферы: гранулы поверх крупных ячеек
                    double t = clamp01(0.55 * g + 0.45 * m);
                    t = sstep(0.30, 0.75, t);
                    Cell col = ramp(sm.cool, sm.mid, sm.warm, t);

                    // факелы: еле заметное осветление, без крупных светлых пятен
                    double faculae = sstep(0.66, 0.82, f) * (0.2 + 0.8 * zone);
                    col = col.mix(sm.hot, faculae * 0.35);

                    // пятна: редкие, небольшие, с широким плавным краем и негустым ядром
                    double s   = spots.get_cell(i, j) - 0.12 * (1.0 - zone);
                    double pen = sstep(thr,        thr + 0.05, s);
                    double umb = sstep(thr + 0.04, thr + 0.09, s);
                    col = col.mix(sm.penum, pen * 0.6);
                    col = col.mix(sm.umbra, umb * 0.7);

                    cells[j][i] = col;
                    water[j][i] = 0;
                    clouds[j][i] = 0;
                }
            }
        }

        // Корона: добавляется к уже нарисованному кадру.
        //  - поверх более далёких тел (в том числе другого солнца) она светится;
        //  - тела, стоящие ближе Солнца, её перекрывают, а вокруг них корона остаётся видна (затмение);
        //  - на самом диске Солнца не рисуется, там уже есть своё свечение края.
        void draw_glow(RenderContext& ctx) override {
            const Camera& cam = ctx.cam;
            Framebuffer& fb = ctx.fb;
            Vec3 c = pos - cam.pos;
            double d = c.len();
            if (c.z <= 0 || d <= radius) return;

            double rd   = cam.focal * tan(asin(radius / d));      // радиус диска на экране
            double cxp  = cam.w / 2.0 + cam.focal * c.x / c.z;
            double cyp  = cam.h / 2.0 - cam.focal * c.y / c.z;
            double reach = rd * (1.0 + corona);
            double front = d - radius - 1e-6;                     // ближайшая точка Солнца

            int x0 = (int)max(0.0, floor(cxp - reach));
            int y0 = (int)max(0.0, floor(cyp - reach));
            int x1 = (int)min((double)cam.w, ceil(cxp + reach));
            int y1 = (int)min((double)cam.h, ceil(cyp + reach));
            x0 -= x0 % dx;
            y0 -= y0 % dx;

            for (int py = y0; py < y1; py += dx){
                for (int px = x0; px < x1; px += dx){
                    double ox = px + dx / 2.0 - cxp;
                    double oy = py + dx / 2.0 - cyp;
                    double dd = max(0.0, sqrt(ox * ox + oy * oy) / rd - 1.0);   // расстояние от края в радиусах
                    double a  = atan2(-oy, ox);

                    // лучи короны медленно меняются с вращением
                    double streak = 0.8 + 0.2 * sin(5 * a + rot * 0.5) * sin(3 * a - rot * 0.3);
                    double glow = (exp(-dd * 6.0) * 0.85 + exp(-dd * 1.8) * 0.18) * streak;

                    // плавное затухание до нуля к границе области (dd == corona)
                    glow *= 1.0 - sstep(corona * 0.35, corona, dd);
                    if (glow < 0.004) continue;
                    glow = min(1.0, glow);

                    double gr = 255 * glow, gg = 190 * glow, gb = 110 * glow * glow;

                    for (int yy = py; yy < py + dx; yy++){
                        for (int xx = px; xx < px + dx; xx++){
                            if (!fb.inside(xx, yy)) continue;
                            size_t i = (size_t)yy * fb.w + xx;
                            if (fb.id[i] == uid) continue;       // собственный диск
                            if (fb.z[i] < front) continue;       // перед Солнцем что-то стоит
                            fb.add(xx, yy, gr, gg, gb);
                        }
                    }
                }
            }
        }
};
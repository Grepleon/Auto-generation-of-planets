#pragma once
#include "sun.cpp"

// Чёрная дыра Шварцшильда.
//
//   radius  - радиус горизонта событий r_s (в мировых единицах). Тень на экране ~2.6 r_s.
//   tilt    - наклон диска: 0 = ребром к камере (если камера в плоскости диска), PI/2 = плашмя.
//   rot     - "время": вращение рисунка диска (внутренние части крутятся быстрее).
//
// Как это работает: сам шар не рисуется. После того как в буфер кадра попали все остальные тела,
// для каждого пикселя вокруг дыры интегрируется траектория фотона в метрике Шварцшильда
//     d²u/dφ² = -u + 1.5 u²,   u = r_s / r
// (уравнение Бине). Луч может: упасть за горизонт (чёрный пиксель), пересечь плоскость диска
// (добавить его свет, в том числе с линзированных вторичных изображений) или уйти на
// бесконечность под изменённым углом - тогда цвет берётся из кадра в отклонённом направлении.
class BlackHole : public Sun {
    private:
        double disk_in, disk_out;     // границы диска в радиусах горизонта (ISCO = 3)
        double full_zone, lens_zone;  // до full_zone линзирование полное, к lens_zone плавно гаснет
        int spin_dir;                 // направление вращения диска: +1 / -1

        Cell c_hot  = Cell(255, 248, 235);
        Cell c_mid  = Cell(255, 165, 55);
        Cell c_cool = Cell(140, 45, 10);

    public:
        BlackHole(int x, int y, int dx, Drawer& draw, Vec3 pos = Vec3(0, 0, 0), double radius = 1.0,
                  double disk_in = 3.0, double disk_out = 9.0, double intensity = 0.5,
                  int spin_dir = 1, double lens_zone = 20.0)
            : Sun(x, y, dx, draw, pos, radius, 0.0, 0.0, intensity),
              disk_in(disk_in), disk_out(disk_out),
              full_zone(disk_out + 2.0), lens_zone(max(lens_zone, disk_out + 4.0)),
              spin_dir(spin_dir) {}

        // Карта турбулентности диска (оттенки серого), берётся по направлению на сфере
        void fill() override {
            Perlin fine   = Perlin(x, y); fine.fill_sphere(5, 0.55, max(6.0, x / 14.0));
            Perlin coarse = Perlin(x, y); coarse.fill_sphere(4, 0.5, 3.0);

            for (int j = 0; j < y; j++){
                for (int i = 0; i < x; i++){
                    double v = clamp01(0.6 * fine.get_cell(i, j) + 0.4 * coarse.get_cell(i, j));
                    v = sstep(0.25, 0.80, v);
                    int c = (int)(255 * v);
                    cells[j][i]  = Cell(c, c, c);
                    water[j][i]  = 0;
                    clouds[j][i] = 0;
                }
            }
        }

        // Шар не рисуется - всё делает post_process
        void show3D(RenderContext& ctx) override {}
        void draw_glow(RenderContext& ctx) override {}

        void post_process(RenderContext& ctx) override {
            const double PI = acos(-1.0);
            const Camera& cam = ctx.cam;
            Framebuffer& fb = ctx.fb;

            Vec3 oc = pos - cam.pos;                 // от камеры к центру
            double d  = oc.len();
            double rs = radius;
            if (oc.z <= 0 || d <= rs * 1.01) return;

            // ---------- область экрана, где искажения заметны ----------
            double zone  = lens_zone * rs;
            double a     = asin(min(1.0, zone / d));
            double theta = acos(max(-1.0, min(1.0, oc.z / d)));
            double cxp = cam.w / 2.0 + cam.focal * oc.x / oc.z;
            double cyp = cam.h / 2.0 - cam.focal * oc.y / oc.z;
            double rpx = (theta + a >= PI / 2 - 0.02)
                ? 1e7
                : cam.focal * max(tan(theta + a) - tan(theta), tan(theta) - tan(theta - a)) * 1.1 + 2;

            int x0 = (int)max(0.0, floor(cxp - rpx));
            int y0 = (int)max(0.0, floor(cyp - rpx));
            int x1 = (int)min((double)cam.w, ceil(cxp + rpx));
            int y1 = (int)min((double)cam.h, ceil(cyp + rpx));
            x0 -= x0 % dx;
            y0 -= y0 % dx;

            // ---------- геометрия диска ----------
            double ct = cos(tilt), st = sin(tilt);
            Vec3 n(0, ct, -st);                      // нормаль диска
            Vec3 u1(1, 0, 0);
            Vec3 u2 = n.cross(u1);                   // оси в плоскости диска
            Vec3 e1 = (cam.pos - pos) * (1.0 / d);   // от центра к камере
            double front = d - disk_out * rs;        // что стоит ближе - дырой не искажается

            vector<float> snap = fb.col;             // кадр до искажения

            // билинейная выборка кадра; тела ближе дыры считаются не фоном
            auto sample = [&](const Vec3& dir, double& r, double& g, double& b){
                r = fb.bgc[0];  g = fb.bgc[1];  b = fb.bgc[2];
                if (dir.z < 0.02) return;
                double sx = cam.w / 2.0 + cam.focal * dir.x / dir.z;
                double sy = cam.h / 2.0 - cam.focal * dir.y / dir.z;
                if (sx < 0 || sy < 0 || sx >= cam.w - 1 || sy >= cam.h - 1) return;
                size_t ni = (size_t)(int)sy * fb.w + (int)sx;
                if (fb.z[ni] < d) return;
                double fx = sx - 0.5, fy = sy - 0.5;
                int ix = max(0, (int)floor(fx)), iy = max(0, (int)floor(fy));
                int jx = min(fb.w - 1, ix + 1), jy = min(fb.h - 1, iy + 1);
                double tx = clamp01(fx - ix), ty = clamp01(fy - iy);
                size_t i00 = (size_t)iy * fb.w + ix, i10 = (size_t)iy * fb.w + jx;
                size_t i01 = (size_t)jy * fb.w + ix, i11 = (size_t)jy * fb.w + jx;
                for (int k = 0; k < 3; k++){
                    double v = snap[3 * i00 + k] * (1 - tx) * (1 - ty) + snap[3 * i10 + k] * tx * (1 - ty)
                             + snap[3 * i01 + k] * (1 - tx) * ty       + snap[3 * i11 + k] * tx * ty;
                    if (k == 0) r = v; else if (k == 1) g = v; else b = v;
                }
            };

            for (int py = y0; py < y1; py += dx){
                for (int px = x0; px < x1; px += dx){
                    if (!fb.inside(px, py)) continue;

                    Vec3 D = cam.ray(px + dx / 2.0, py + dx / 2.0);
                    double along = oc.dot(D);
                    if (along <= 0) continue;
                    double bimp = (D * along - oc).len() / rs;       // прицельный параметр в r_s
                    if (bimp > lens_zone) continue;
                    if (fb.z[(size_t)py * fb.w + px] < front) continue;

                    // плоскость луча: e1 (от центра к камере) и e2 (поперёк, в сторону луча)
                    double cosA = -D.dot(e1);
                    Vec3 pr = D + e1 * cosA;
                    double sinA = pr.len();

                    double ar = 0, ag = 0, ab = 0, trans = 1.0;
                    bool captured = false, escaped = false;
                    double phi_f = 0;
                    Vec3 e2;

                    if (sinA < 1e-9){
                        if (cosA > 0) captured = true; else continue;
                    } else {
                        e2 = pr * (1.0 / sinA);
                        double ca = n.dot(e1), cb = n.dot(e2);

                        double u = rs / d;
                        double v = u * cosA / sinA;                   // du/dφ
                        double phi = 0, g_prev = ca;
                        bool done = false;

                        for (int step = 0; step < 2500 && !done; step++){
                            // шаг мельче у дыры, крупнее вдали
                            double h = (u > 0.08) ? 0.02 : (u > 0.03 ? 0.05 : 0.12);

                            double k1u = v,  k1v = -u + 1.5 * u * u;
                            double u2_ = u + 0.5 * h * k1u, v2_ = v + 0.5 * h * k1v;
                            double k2u = v2_, k2v = -u2_ + 1.5 * u2_ * u2_;
                            double u3_ = u + 0.5 * h * k2u, v3_ = v + 0.5 * h * k2v;
                            double k3u = v3_, k3v = -u3_ + 1.5 * u3_ * u3_;
                            double u4_ = u + h * k3u,       v4_ = v + h * k3v;
                            double k4u = v4_, k4v = -u4_ + 1.5 * u4_ * u4_;
                            double un = u + h / 6.0 * (k1u + 2 * k2u + 2 * k3u + k4u);
                            double vn = v + h / 6.0 * (k1v + 2 * k2v + 2 * k3v + k4v);
                            double phin = phi + h;

                            if (un <= 0){                              // ушёл на бесконечность
                                phi_f = phi + h * (u / (u - un));
                                escaped = true;  done = true;
                                break;
                            }
                            if (un >= 1.0){                            // упал за горизонт
                                captured = true;  done = true;
                                break;
                            }

                            // пересечение плоскости диска: высота ~ r * (cosφ * ca + sinφ * cb)
                            double gn = cos(phin) * ca + sin(phin) * cb;
                            if (gn * g_prev < 0 && trans > 0.01){
                                double s  = g_prev / (g_prev - gn);
                                double pc = phi + h * s;
                                double uc = u + (un - u) * s;
                                double rc = 1.0 / uc;                  // радиус в r_s

                                if (rc > disk_in && rc < disk_out){
                                    Vec3 pd = e1 * cos(pc) + e2 * sin(pc);   // направление от центра
                                    double psi = atan2(pd.dot(u2), pd.dot(u1));

                                    // рисунок диска: внутренние части вращаются быстрее (кеплер)
                                    double psi2 = psi - rot * spin_dir * pow(rc, -1.5) * 3.0;
                                    double zq = log(rc / disk_in) * 0.55 + 0.1;
                                    double qn = sqrt(1.0 + zq * zq);
                                    double lon = atan2(sin(psi2), cos(psi2));
                                    double lat = asin(zq / qn);
                                    double fx, fy, tr, tg, tb;
                                    to_map(lon, lat, fx, fy);
                                    sample_color(fx, fy, tr, tg, tb);
                                    double nz = tr / 255.0;

                                    // температура ~ r^-3/4
                                    double tprof = pow(disk_in / rc, 0.75);

                                    // эффект Доплера и гравитационное красное смещение
                                    double vel = sqrt(0.5 / rc);                  // скорость орбиты, c = 1
                                    Vec3 tang = n.cross(pd) * (double)spin_dir;
                                    Vec3 P = pos + pd * (rc * rs);
                                    Vec3 kdir = (cam.pos - P).norm();
                                    double vk  = tang.dot(kdir) * vel;
                                    double gam = 1.0 / sqrt(1.0 - vel * vel);
                                    double dop = 1.0 / (gam * (1.0 - vk));
                                    double grav = sqrt(max(0.05, 1.0 - 1.0 / rc));
                                    double gt = dop * grav;

                                    // сторона, летящая к нам, ярче и белее
                                    double bright = min(1.8, pow(tprof, 1.2) * pow(gt, 3.0));
                                    double tt = clamp01((tprof * gt - 0.25) / 0.9);
                                    Cell col = ramp(c_cool, c_mid, c_hot, tt);
                                    int* cc = col.get_color();

                                    double mod = (0.55 + 0.9 * nz) * (1.0 + 0.12 * sin(rc * 9.0));
                                    double alpha = sstep(disk_in, disk_in + 0.35, rc)
                                                 * (1.0 - sstep(disk_out - 2.5, disk_out, rc)) * 0.92;

                                    double k = bright * mod;
                                    ar += trans * alpha * cc[0] * k;
                                    ag += trans * alpha * cc[1] * k;
                                    ab += trans * alpha * cc[2] * k;
                                    trans *= (1.0 - alpha);
                                }
                            }

                            g_prev = gn;  u = un;  v = vn;  phi = phin;
                        }
                        if (!done) captured = true;                    // застрял на фотонной сфере
                    }

                    // ---------- фон: куда в итоге смотрит луч ----------
                    double br = 0, bg = 0, bb = 0;
                    if (escaped){
                        Vec3 Dout = e1 * cos(phi_f) + e2 * sin(phi_f);
                        double win = 1.0 - sstep(full_zone, lens_zone, bimp);   // к краю области искажение гаснет
                        Vec3 Dw = (D + (Dout - D) * win).norm();
                        sample(Dw, br, bg, bb);
                    }
                    // captured: фон чёрный; трасса, не нашедшая ни диска, ни отклонения, пиксель не меняет
                    if (!captured && trans > 0.999 && escaped){
                        double dev = (D - (e1 * cos(phi_f) + e2 * sin(phi_f))).len();
                        if (dev < 1e-6) continue;
                    }

                    double rr = ar + trans * br;
                    double gg = ag + trans * bg;
                    double bl = ab + trans * bb;
                    rr = min(255.0, rr);  gg = min(255.0, gg);  bl = min(255.0, bl);

                    for (int yy = py; yy < py + dx; yy++)
                        for (int xx = px; xx < px + dx; xx++)
                            fb.set(xx, yy, rr, gg, bl);
                }
            }
        }
};
#pragma once
#include "planet.cpp"
#include "sun.cpp"

// Сцена: камера + набор тел (планеты, солнца). Источники света собираются автоматически
// из тех тел, у которых emit_light() возвращает true.
//
// Порядок кадра:
//   1. все тела рисуются в буфер с проверкой глубины (перекрытия считаются попиксельно,
//      поэтому сортировка по расстоянию не нужна и пересекающиеся тела не ломаются);
//   2. короны складываются с кадром и скрываются телами, стоящими ближе;
//   3. постобработка (гравитационное линзирование чёрных дыр);
//   4. буфер переносится на холст.
class Scene {
    private:
        Camera cam;
        Drawer& draw;
        Cell bg;                       // цвет фона, на который накладывается свечение
        vector<Planet*> bodies;

    public:
        Scene(const Camera& cam, Drawer& draw, Cell bg = Cell(0, 0, 0))
            : cam(cam), draw(draw), bg(bg) {}

        // тело остаётся во владении вызывающего кода
        void add(Planet* p){
            p->set_id((int)bodies.size());
            bodies.push_back(p);
        }

        Camera& camera(){ return cam; }

        void render(){
            int* bc = bg.get_color();
            RenderContext ctx(cam, bc[0], bc[1], bc[2]);

            for (Planet* b : bodies){
                ctx.bodies.push_back(b);
                Light l;
                if (b->emit_light(l)) ctx.lights.push_back(l);
            }

            for (Planet* b : bodies) b->show3D(ctx);
            for (Planet* b : bodies) b->draw_glow(ctx);
            for (Planet* b : bodies) b->post_process(ctx);

            // перенос на холст: только изменённые пиксели
            Framebuffer& fb = ctx.fb;
            for (int py = 0; py < fb.h; py++){
                for (int px = 0; px < fb.w; px++){
                    size_t i = (size_t)py * fb.w + px;
                    if (!fb.dirty[i]) continue;
                    Cell out((int)min(255.0f, fb.col[3 * i]),
                             (int)min(255.0f, fb.col[3 * i + 1]),
                             (int)min(255.0f, fb.col[3 * i + 2]));
                    draw.rectangle(px, py, px + 1, py + 1, out.get_color());
                }
            }
        }
};
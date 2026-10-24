#pragma once
#include "planet.cpp"
#include "sun.cpp"

// Сцена: камера + набор тел (планеты, солнца). Источники света собираются автоматически
// из тех тел, у которых emit_light() возвращает true.
class Scene {
    private:
        Camera cam;
        vector<Planet*> bodies;

    public:
        Scene(const Camera& cam): cam(cam) {}

        // тело остаётся во владении вызывающего кода
        void add(Planet* p){ bodies.push_back(p); }

        Camera& camera(){ return cam; }

        void render(){
            RenderContext ctx(cam);

            for (Planet* b : bodies){
                ctx.bodies.push_back(b);
                Light l;
                if (b->emit_light(l)) ctx.lights.push_back(l);
            }

            // 1. свечение и короны - позади всего
            for (Planet* b : bodies) b->draw_back(ctx);

            // 2. тела от дальнего к ближнему
            vector<Planet*> order = bodies;
            Vec3 cp = cam.pos;
            sort(order.begin(), order.end(), [&](Planet* a, Planet* b){
                return (a->get_pos() - cp).len() > (b->get_pos() - cp).len();
            });
            for (Planet* b : order) b->show3D(ctx);
        }
};
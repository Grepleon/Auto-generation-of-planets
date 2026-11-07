#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "draw.cpp"
#include "objects/planet.cpp"
#include "objects/planet_type.cpp"
#include "objects/sun.cpp"
#include "objects/blackhole.cpp"
#include "objects/scene.cpp"
#include <vector>
#include <memory>
#include <string>

using namespace std;

// Создаёт планету нужного типа, заполняет карту и задаёт ориентацию
static TypedPlanet* make(vector<unique_ptr<TypedPlanet>>& keep, PlanetType t,
                         int mw, int mh, Drawer& draw, Vec3 pos, double r,
                         double rot, double tilt, unsigned seed){
    keep.emplace_back(new TypedPlanet(t, mw, mh, 1, draw, pos, r, seed));
    TypedPlanet* p = keep.back().get();
    p->fill();
    p->set_pose(rot, tilt);
    return p;
}

// ---------------------------------------------------------------
//  1) Галерея: все типы планет в сетке 11 x 3
// ---------------------------------------------------------------
static void gallery(Drawer& draw, int W, int H, const string& out){
    Camera cam(W, H, Vec3(0, 0, -60), 40);
    Scene scene(cam, draw);

    // Солнце за камерой слева сверху: освещает всех, само в кадр не попадает
    Sun sun(256, 128, 1, draw, Vec3(-90, 60, -150), 10.0, 0.0, 0.0, 1.1);
    sun.fill();
    scene.add(&sun);

    vector<unique_ptr<TypedPlanet>> keep;
    int n = (int)PlanetType::COUNT, cols = 11;
    for (int k = 0; k < n; k++){
        int c = k % cols, r = k / cols;
        Vec3 pos((c - 5) * 6.9, (1 - r) * 8.4, 0);
        double tilt = 0.25 + 0.2 * sin(k);
        PlanetType t = (PlanetType)k;
        if (t == PlanetType::GAS_SATURN) tilt = 1.1;           // кольца видны под углом
        TypedPlanet* p = make(keep, t, 768, 384, draw, pos, 3.0, k * 0.9, tilt, k + 1);
        if (t == PlanetType::GAS_SATURN) p->set_ring_range(1.25, 1.65);
        scene.add(p);
    }
    scene.render();
    draw.Save(out.c_str());
}

// ---------------------------------------------------------------
//  2) Звёздная система: Солнце слева, разные миры по порядку
// ---------------------------------------------------------------
static void system_scene(Drawer& draw, int W, int H, const string& out){
    Camera cam(W, H, Vec3(0, 0, -60), 40);
    Scene scene(cam, draw);

    Sun sun(1024, 512, 1, draw, Vec3(-52, 4, 25), 9.0, 0.6, 0.3, 1.0);
    sun.fill();
    scene.add(&sun);

    vector<unique_ptr<TypedPlanet>> keep;
    auto add = [&](PlanetType t, double x, double y, double z, double r,
                   double rot, double tilt, unsigned seed){
        TypedPlanet* p = make(keep, t, 1024, 512, draw, Vec3(x, y, z), r, rot, tilt, seed);
        scene.add(p);
        return p;
    };

    add(PlanetType::LAVA,        -38, -12,  0, 1.5, 0.5, 0.2, 11);
    add(PlanetType::MERCURY,     -30,  -5,  3, 1.1, 1.0, 0.1, 12);
    add(PlanetType::VENUS,       -18,   6,  2, 1.9, 2.0, 0.1, 13);
    add(PlanetType::EARTH,        -3,  -4,  0, 2.4, 1.2, 0.4, 14);
    add(PlanetType::MOON,          4,   3, -1.5, 0.65, 0.3, 0.1, 15);
    add(PlanetType::MARS,         10,   6,  3, 1.7, 1.2, 0.4, 16);
    add(PlanetType::EUROPA,       13,  -7,  0, 0.9, 0.7, 0.2, 17);
    add(PlanetType::ICE_NEPTUNE, -10, -14, 10, 3.0, 0.4, 0.3, 18);
    add(PlanetType::GAS_JUPITER,  22,  -3, 12, 6.0, 0.8, 0.1, 19);
    add(PlanetType::GAS_SATURN,   31,  12, 18, 4.0, 0.5, 1.0, 20);
    add(PlanetType::TITAN,        34, -14,  6, 1.3, 0.2, 0.2, 21);

    scene.render();
    draw.Save(out.c_str());
}

// ---------------------------------------------------------------
//  3) Чёрная дыра и экзотические миры вокруг (твоя исходная сцена)
// ---------------------------------------------------------------
static void blackhole_scene(Drawer& draw, int W, int H, const string& out){
    Camera cam(W, H, Vec3(0, 0, -60), 40);
    Scene scene(cam, draw);

    BlackHole bh(1024, 512, 1, draw, Vec3(0, 0, 40), 2.0);
    bh.fill();
    bh.set_pose(0.0, 0.2);
    scene.add(&bh);

    // слабый заполняющий свет из-за камеры, чтобы миры не были совсем чёрными
    Sun fill_light(256, 128, 1, draw, Vec3(-70, 40, -80), 5.0, 0.0, 0.0, 0.8);
    fill_light.fill();
    scene.add(&fill_light);

    vector<unique_ptr<TypedPlanet>> keep;
    auto add = [&](PlanetType t, double x, double y, double z, double r,
                   double rot, double tilt, unsigned seed){
        TypedPlanet* p = make(keep, t, 1024, 512, draw, Vec3(x, y, z), r, rot, tilt, seed);
        scene.add(p);
        return p;
    };

    add(PlanetType::SHATTERED,   10,  -2,  5, 3.0, 1.2, 0.4, 31);
    add(PlanetType::CRYSTAL,     -1, -12,  1, 2.0, 1.0, 0.3, 32);
    add(PlanetType::GAS_SATURN, -16,   8, 10, 3.5, 0.5, 1.0, 33);
    add(PlanetType::EYEBALL,     17,  10, 20, 2.5, 0.0, 0.3, 34);
    add(PlanetType::CITY_WORLD, -18, -9,  2, 2.2, 1.0, 0.3, 35);

    // кольца можно добавить любой планете
    TypedPlanet* ringed = add(PlanetType::ICE_URANUS, 24, -10, 8, 2.4, 0.3, 1.2, 36);
    ringed->set_ring(1.4, 2.0, Cell(190, 220, 225), Cell(120, 150, 160), 0.6);

    scene.render();
    draw.Save(out.c_str());
}

int main(int argc, char** argv) {
    int W = 2560, H = 1440;
    string mode = argc > 1 ? argv[1] : "gallery";   // gallery | system | blackhole

    Drawer draw = Drawer(W, H);
    draw.blackRectangle(0, 0, W - 1, H - 1);

    if      (mode == "system")    system_scene(draw, W, H, "data/planet/system.png");
    else if (mode == "blackhole") blackhole_scene(draw, W, H, "data/planet/blackhole.png");
    else                          gallery(draw, W, H, "data/planet/gallery.png");
}
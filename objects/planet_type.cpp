#pragma once
#include "planet.cpp"
#include <random>

// ============================================================
//  Типы планет
// ============================================================
//  Планета выбирается одной строкой:
//      TypedPlanet p(PlanetType::LAVA, 1024, 512, 1, draw, Vec3(0,0,0), 2.0);
//      p.fill();
//  Каждый тип - это "профиль" (make_profile): палитра, океаны, лёд, кратеры, трещины,
//  облака, атмосфера, свечение, полосы газового гиганта, кольца...
//  Чтобы добавить свой тип: допиши имя в enum, заведи case в make_profile.

enum class PlanetType {
    // --- каменные ---
    EARTH,        // Земля: биомы, океаны, облака (оригинальная генерация Planet::fill)
    MARS,         // Марс: ржавая пустыня, тёмные равнины, пыль, полярные шапки
    DESERT,       // Пустынная планета (Татуин / Арракис)
    OCEAN,        // Планета-океан с редкими островами
    JUNGLE,       // Джунглевая: густая зелень, влажные облака
    TUNDRA,       // Холодная земля: льды, редкие моря
    ALIEN,        // Чужая биосфера: фиолетовая растительность, бирюзовые моря
    LAVA,         // Лавовая: магматические моря, светящиеся трещины
    ICE_WORLD,    // Ледяной шар (снежок)
    MERCURY,      // Меркурий: серые кратеры без атмосферы
    VENUS,        // Венера: сплошные жёлтые облака
    TOXIC,        // Токсичная: кислотные зелёные моря и облака
    CITY_WORLD,   // Экуменополис: планета-город, ночные огни
    EUROPA,       // Ледяной спутник с трещинами
    MOON,         // Луна: кратеры и тёмные моря
    IO,           // Ио: серная вулканическая
    TITAN,        // Титан: оранжевая плотная дымка
    DWARF,        // Плутон: карликовая планета со светлой равниной
    CARBON,       // Углеродная: графит и алмазные хребты
    IRON,         // Железная: ржавый металл
    CRYSTAL,      // Кристаллическая: фиолетовые грани, светящиеся жилы
    EYEBALL,      // "Глаз": приливный захват, горячая сторона / море / лёд
    ROGUE,        // Планета-изгой: тёмный замёрзший мир
    RADIOACTIVE,  // Радиоактивная: ядовито-зелёные светящиеся жилы
    SHATTERED,    // Расколотая: раскалённые разломы в коре
    // --- гиганты ---
    GAS_JUPITER,  // Юпитер: пояса и Красное пятно
    GAS_SATURN,   // Сатурн: мягкие полосы и кольца
    ICE_URANUS,   // Ледяной гигант (Уран): почти гладкий, бирюзовый
    ICE_NEPTUNE,  // Ледяной гигант (Нептун): синий, тёмные пятна, перистые облака
    HOT_JUPITER,  // Горячий юпитер: чёрный и раскалённый
    BROWN_DWARF,  // Коричневый карлик: тлеющий
    GAS_TEAL,     // Бирюзово-зелёный гигант
    GAS_VIOLET,   // Фиолетово-розовый гигант
    COUNT
};

struct RGB { double r = 0, g = 0, b = 0; };
static inline RGB to_rgb(Cell c){
    int* k = c.get_color();
    RGB o;  o.r = k[0];  o.g = k[1];  o.b = k[2];
    return o;
}

struct PlanetProfile {
    const char* name = "planet";
    enum Family { ROCKY, GAS, EARTH_FAMILY } family = ROCKY;

    // --- твёрдая поверхность: низины -> середина -> высоты ---
    Cell low  = Cell(90, 80, 70);
    Cell mid  = Cell(130, 115, 100);
    Cell high = Cell(170, 160, 150);
    Cell alt  = Cell(60, 55, 50);        double alt_amount = 0;     // пятна другого грунта (тёмные равнины)
    Cell ridge_col = Cell(150, 140, 130); double ridge_amount = 0;  // цвет хребтов
    double relief_freq = 1.4;            // масштаб рельефа
    double mount = 0.18;                 // высота хребтов
    double contrast = 3.0;               // насколько быстро цвет идёт от низин к высотам
    double relief_shade = 0.15;          // "запечённая" тень рельефа

    // --- жидкость ---
    double sea_frac = 0;                 // доля поверхности под жидкостью (0 = сухая)
    Cell shore  = Cell(30, 110, 125);
    Cell liquid = Cell(12, 55, 100);
    Cell deep   = Cell(4, 20, 50);
    double liquid_emit = 0;              // жидкость светится (лава)
    double gloss = 1.0;                  // блик на жидкости

    // --- лёд ---
    double ice_cap = 0;                  // 0 - нет, 1 - шапки до ~45° ширины, 2 - вся планета
    Cell ice = Cell(235, 240, 245);

    // --- "глаз" ---
    bool eyeball = false;
    Cell hot_col = Cell(120, 50, 30);
    double heat_emit = 0;

    // --- детали ---
    double crater_density = 0, crater_size = 1.0;
    double vents = 0;                    // вулканические пятна (Ио)
    double cracks = 0;                   // трещины / разломы
    Cell crack_col = Cell(40, 30, 30);
    double crack_emit = 0;               // светятся ли трещины
    double cities = 0;                   // огни городов (1 - везде)

    // --- свечение ---
    Cell glow_col = Cell(255, 150, 50);
    bool glow_night_only = false;        // светится только на ночной стороне (города)
    double uniform_glow = 0;             // равномерное собственное свечение (раскалённые тела)

    // --- облака и атмосфера ---
    double cloud_cover = 0;              // 0..1
    Cell cloud_col = Cell(245, 247, 250);
    double cloud_alpha = 0.9;
    Cell atm_col = Cell(70, 130, 255);
    double atm_strength = 0.8;           // 0 - без атмосферы
    double wrap = 0;                     // мягкость терминатора (плотная атмосфера)
    double ambient = 0.04;

    // --- газовые планеты ---
    Cell g1 = Cell(120, 90, 60), g2 = Cell(190, 150, 110), g3 = Cell(235, 220, 190);
    double bands = 10, turb = 0.5, band_contrast = 1.0;
    int storms = 0;
    Cell storm_col = Cell(190, 80, 50);
    double storm_size = 0.12, storm_alpha = 0.85;

    // --- кольца ---
    bool ring = false;
    double ring_in = 1.3, ring_out = 2.2;
    Cell ring_a = Cell(215, 195, 160), ring_b = Cell(140, 120, 100);
    double ring_alpha = 0.85;
};

static PlanetProfile make_profile(PlanetType t){
    PlanetProfile p;
    switch (t){
    case PlanetType::EARTH:
        p.name = "Earth";  p.family = PlanetProfile::EARTH_FAMILY;
        break;

    case PlanetType::MARS:
        p.name = "Mars";
        p.low = Cell(95, 45, 30);  p.mid = Cell(165, 85, 50);  p.high = Cell(205, 130, 85);
        p.alt = Cell(70, 40, 35);  p.alt_amount = 0.8;
        p.ridge_col = Cell(120, 70, 55);  p.ridge_amount = 0.4;
        p.contrast = 2.5;  p.relief_freq = 1.6;  p.relief_shade = 0.2;
        p.ice_cap = 0.25;  p.ice = Cell(240, 235, 235);
        p.crater_density = 0.35;
        p.cloud_cover = 0.12;  p.cloud_col = Cell(225, 190, 165);  p.cloud_alpha = 0.5;
        p.atm_col = Cell(210, 150, 110);  p.atm_strength = 0.35;
        break;

    case PlanetType::DESERT:
        p.name = "Desert";
        p.low = Cell(170, 125, 70);  p.mid = Cell(215, 170, 105);  p.high = Cell(238, 208, 150);
        p.alt = Cell(150, 95, 55);  p.alt_amount = 0.5;
        p.ridge_col = Cell(120, 85, 60);  p.ridge_amount = 0.5;
        p.contrast = 2.5;  p.relief_freq = 1.8;
        p.crater_density = 0.05;
        p.cloud_cover = 0.1;  p.cloud_alpha = 0.6;
        p.atm_col = Cell(235, 195, 130);  p.atm_strength = 0.5;
        break;

    case PlanetType::OCEAN:
        p.name = "Ocean";
        p.sea_frac = 0.92;
        p.shore = Cell(60, 165, 180);  p.liquid = Cell(15, 85, 150);  p.deep = Cell(5, 30, 85);
        p.low = Cell(190, 180, 130);  p.mid = Cell(60, 135, 60);  p.high = Cell(110, 110, 100);
        p.ice_cap = 0.5;  p.cloud_cover = 0.55;  p.gloss = 1.3;
        break;

    case PlanetType::JUNGLE:
        p.name = "Jungle";
        p.sea_frac = 0.42;
        p.shore = Cell(40, 150, 140);  p.liquid = Cell(15, 90, 120);  p.deep = Cell(5, 40, 70);
        p.low = Cell(25, 95, 40);  p.mid = Cell(25, 140, 50);  p.high = Cell(80, 125, 65);
        p.alt = Cell(12, 65, 32);  p.alt_amount = 0.5;
        p.ice_cap = 0.2;  p.cloud_cover = 0.55;
        p.atm_col = Cell(90, 175, 240);
        break;

    case PlanetType::TUNDRA:
        p.name = "Tundra";
        p.sea_frac = 0.3;
        p.shore = Cell(70, 120, 140);  p.liquid = Cell(25, 60, 100);  p.deep = Cell(8, 25, 55);
        p.low = Cell(115, 120, 110);  p.mid = Cell(170, 180, 175);  p.high = Cell(238, 242, 246);
        p.ice_cap = 1.0;  p.cloud_cover = 0.5;
        p.atm_col = Cell(150, 190, 255);
        break;

    case PlanetType::ALIEN:
        p.name = "Alien biosphere";
        p.sea_frac = 0.4;
        p.shore = Cell(90, 205, 205);  p.liquid = Cell(30, 120, 175);  p.deep = Cell(10, 50, 105);
        p.low = Cell(105, 40, 130);  p.mid = Cell(170, 60, 170);  p.high = Cell(225, 135, 205);
        p.alt = Cell(55, 20, 100);  p.alt_amount = 0.5;
        p.ice_cap = 0.2;
        p.cloud_cover = 0.4;  p.cloud_col = Cell(255, 228, 246);  p.cloud_alpha = 0.8;
        p.atm_col = Cell(230, 120, 255);  p.atm_strength = 0.9;
        break;

    case PlanetType::LAVA:
        p.name = "Lava";
        p.sea_frac = 0.35;
        p.shore = Cell(255, 170, 40);  p.liquid = Cell(255, 100, 15);  p.deep = Cell(190, 40, 5);
        p.liquid_emit = 1.0;  p.gloss = 0;
        p.low = Cell(25, 20, 20);  p.mid = Cell(48, 37, 34);  p.high = Cell(85, 68, 62);
        p.ridge_col = Cell(30, 25, 25);  p.ridge_amount = 0.3;
        p.cracks = 1.5;  p.crack_col = Cell(255, 120, 30);  p.crack_emit = 1.0;
        p.glow_col = Cell(255, 110, 25);
        p.crater_density = 0.15;
        p.cloud_cover = 0.35;  p.cloud_col = Cell(75, 58, 52);  p.cloud_alpha = 0.65;
        p.atm_col = Cell(255, 110, 40);  p.atm_strength = 0.9;
        break;

    case PlanetType::ICE_WORLD:
        p.name = "Ice world";
        p.low = Cell(190, 215, 235);  p.mid = Cell(225, 235, 245);  p.high = Cell(245, 248, 252);
        p.ice_cap = 2.0;  p.ice = Cell(235, 243, 250);
        p.cracks = 0.6;  p.crack_col = Cell(120, 170, 210);
        p.crater_density = 0.1;  p.cloud_cover = 0.25;
        p.atm_col = Cell(150, 200, 255);  p.atm_strength = 0.7;
        break;

    case PlanetType::MERCURY:
        p.name = "Mercury";
        p.low = Cell(80, 76, 72);  p.mid = Cell(125, 118, 110);  p.high = Cell(165, 158, 150);
        p.alt = Cell(95, 90, 88);  p.alt_amount = 0.5;
        p.ridge_col = Cell(70, 66, 63);  p.ridge_amount = 0.2;
        p.contrast = 2.0;  p.crater_density = 0.85;
        p.cracks = 0.3;  p.crack_col = Cell(60, 57, 55);
        p.atm_strength = 0;
        break;

    case PlanetType::VENUS:
        p.name = "Venus";
        p.low = Cell(170, 120, 60);  p.mid = Cell(200, 150, 80);  p.high = Cell(230, 190, 120);
        p.cloud_cover = 1.0;  p.cloud_col = Cell(238, 208, 142);  p.cloud_alpha = 0.95;
        p.atm_col = Cell(255, 210, 120);  p.atm_strength = 0.9;  p.wrap = 0.3;
        break;

    case PlanetType::TOXIC:
        p.name = "Toxic";
        p.sea_frac = 0.5;
        p.shore = Cell(175, 195, 40);  p.liquid = Cell(110, 150, 20);  p.deep = Cell(60, 90, 10);
        p.low = Cell(120, 100, 40);  p.mid = Cell(160, 140, 50);  p.high = Cell(190, 175, 90);
        p.alt = Cell(80, 70, 30);  p.alt_amount = 0.4;
        p.cloud_cover = 0.6;  p.cloud_col = Cell(200, 215, 90);  p.cloud_alpha = 0.75;
        p.atm_col = Cell(190, 230, 60);  p.atm_strength = 0.9;
        break;

    case PlanetType::CITY_WORLD:
        p.name = "City world";
        p.sea_frac = 0.2;
        p.low = Cell(70, 70, 78);  p.mid = Cell(105, 105, 115);  p.high = Cell(150, 150, 160);
        p.cities = 1.0;  p.glow_col = Cell(255, 200, 110);  p.glow_night_only = true;
        p.cloud_cover = 0.4;  p.cloud_col = Cell(175, 165, 145);  p.cloud_alpha = 0.5;
        p.atm_col = Cell(150, 170, 255);
        break;

    case PlanetType::EUROPA:
        p.name = "Europa-like";
        p.low = Cell(200, 185, 160);  p.mid = Cell(225, 215, 195);  p.high = Cell(240, 235, 225);
        p.alt = Cell(190, 160, 130);  p.alt_amount = 0.4;
        p.cracks = 2.0;  p.crack_col = Cell(140, 88, 58);
        p.crater_density = 0.05;  p.contrast = 2.0;  p.relief_shade = 0.05;
        p.atm_strength = 0.05;
        break;

    case PlanetType::MOON:
        p.name = "Moon";
        p.low = Cell(70, 70, 72);  p.mid = Cell(130, 130, 132);  p.high = Cell(180, 180, 182);
        p.alt = Cell(48, 48, 52);  p.alt_amount = 0.9;
        p.contrast = 2.0;  p.crater_density = 0.7;
        p.atm_strength = 0;
        break;

    case PlanetType::IO:
        p.name = "Io-like";
        p.low = Cell(200, 170, 60);  p.mid = Cell(225, 200, 90);  p.high = Cell(238, 228, 150);
        p.alt = Cell(175, 90, 40);  p.alt_amount = 0.6;
        p.vents = 1.0;  p.crater_density = 0.04;  p.glow_col = Cell(255, 110, 30);
        p.atm_col = Cell(160, 200, 255);  p.atm_strength = 0.15;
        break;

    case PlanetType::TITAN:
        p.name = "Titan-like";
        p.low = Cell(150, 95, 35);  p.mid = Cell(185, 125, 50);  p.high = Cell(215, 160, 80);
        p.cloud_cover = 1.0;  p.cloud_col = Cell(215, 150, 60);  p.cloud_alpha = 0.88;
        p.atm_col = Cell(235, 150, 50);  p.atm_strength = 1.1;  p.wrap = 0.35;
        break;

    case PlanetType::DWARF:
        p.name = "Dwarf planet";
        p.low = Cell(110, 80, 60);  p.mid = Cell(160, 125, 95);  p.high = Cell(200, 175, 150);
        p.alt = Cell(240, 232, 222);  p.alt_amount = 0.9;
        p.crater_density = 0.15;  p.cracks = 0.3;  p.crack_col = Cell(80, 55, 45);
        p.atm_col = Cell(120, 150, 225);  p.atm_strength = 0.25;
        break;

    case PlanetType::CARBON:
        p.name = "Carbon";
        p.low = Cell(15, 15, 18);  p.mid = Cell(35, 35, 42);  p.high = Cell(70, 70, 85);
        p.ridge_col = Cell(205, 220, 235);  p.ridge_amount = 0.75;
        p.cracks = 1.0;  p.crack_col = Cell(185, 205, 230);
        p.crater_density = 0.2;
        p.atm_col = Cell(90, 100, 145);  p.atm_strength = 0.2;
        break;

    case PlanetType::IRON:
        p.name = "Iron";
        p.low = Cell(90, 60, 50);  p.mid = Cell(140, 95, 70);  p.high = Cell(175, 135, 105);
        p.alt = Cell(70, 70, 78);  p.alt_amount = 0.6;
        p.ridge_col = Cell(195, 195, 205);  p.ridge_amount = 0.6;
        p.crater_density = 0.4;
        p.atm_strength = 0;
        break;

    case PlanetType::CRYSTAL:
        p.name = "Crystal";
        p.low = Cell(120, 60, 170);  p.mid = Cell(160, 110, 230);  p.high = Cell(212, 192, 255);
        p.alt = Cell(70, 40, 130);  p.alt_amount = 0.5;
        p.ridge_col = Cell(150, 240, 255);  p.ridge_amount = 0.8;
        p.cracks = 1.2;  p.crack_col = Cell(230, 250, 255);  p.crack_emit = 0.4;
        p.glow_col = Cell(160, 230, 255);
        p.contrast = 3.0;  p.mount = 0.3;  p.relief_shade = 0.3;
        p.atm_col = Cell(170, 130, 255);  p.atm_strength = 0.7;
        break;

    case PlanetType::EYEBALL:
        p.name = "Eyeball";  p.eyeball = true;
        p.sea_frac = 0.5;
        p.shore = Cell(70, 180, 190);  p.liquid = Cell(20, 90, 150);  p.deep = Cell(6, 35, 90);
        p.low = Cell(150, 120, 70);  p.mid = Cell(90, 130, 70);  p.high = Cell(150, 150, 130);
        p.hot_col = Cell(125, 52, 30);  p.heat_emit = 0.45;  p.glow_col = Cell(255, 120, 30);
        p.ice = Cell(238, 246, 255);
        p.cloud_cover = 0.45;
        break;

    case PlanetType::ROGUE:
        p.name = "Rogue";
        p.low = Cell(35, 45, 60);  p.mid = Cell(60, 75, 95);  p.high = Cell(100, 120, 145);
        p.ice_cap = 2.0;  p.ice = Cell(75, 95, 120);
        p.cracks = 0.8;  p.crack_col = Cell(20, 25, 35);
        p.uniform_glow = 0.05;  p.glow_col = Cell(60, 100, 200);
        p.atm_col = Cell(70, 100, 180);  p.atm_strength = 0.1;  p.ambient = 0.06;
        break;

    case PlanetType::RADIOACTIVE:
        p.name = "Radioactive";
        p.sea_frac = 0.25;
        p.shore = Cell(140, 255, 60);  p.liquid = Cell(60, 230, 40);  p.deep = Cell(30, 150, 20);
        p.liquid_emit = 0.6;  p.glow_col = Cell(120, 255, 50);
        p.low = Cell(30, 35, 25);  p.mid = Cell(55, 60, 40);  p.high = Cell(90, 95, 60);
        p.cracks = 2.0;  p.crack_col = Cell(140, 255, 60);  p.crack_emit = 1.0;
        p.crater_density = 0.2;
        p.cloud_cover = 0.4;  p.cloud_col = Cell(150, 200, 100);  p.cloud_alpha = 0.5;
        p.atm_col = Cell(140, 255, 80);  p.atm_strength = 0.9;
        break;

    case PlanetType::SHATTERED:
        p.name = "Shattered";
        p.low = Cell(40, 30, 28);  p.mid = Cell(70, 55, 50);  p.high = Cell(110, 90, 80);
        p.cracks = 3.5;  p.crack_col = Cell(255, 170, 60);  p.crack_emit = 1.0;
        p.glow_col = Cell(255, 150, 40);
        p.crater_density = 0.3;
        p.atm_col = Cell(255, 140, 50);  p.atm_strength = 0.5;
        break;

    // ------------------ гиганты ------------------
    case PlanetType::GAS_JUPITER:
        p.name = "Gas giant (Jupiter)";  p.family = PlanetProfile::GAS;
        p.g1 = Cell(150, 100, 70);  p.g2 = Cell(200, 150, 105);  p.g3 = Cell(240, 225, 200);
        p.bands = 14;  p.turb = 0.6;
        p.storms = 3;  p.storm_col = Cell(200, 80, 50);  p.storm_size = 0.16;
        p.atm_col = Cell(200, 170, 130);  p.atm_strength = 0.35;  p.wrap = 0.12;
        break;

    case PlanetType::GAS_SATURN:
        p.name = "Gas giant (Saturn)";  p.family = PlanetProfile::GAS;
        p.g1 = Cell(170, 140, 95);  p.g2 = Cell(215, 190, 140);  p.g3 = Cell(240, 225, 185);
        p.bands = 10;  p.turb = 0.35;  p.band_contrast = 0.8;
        p.atm_col = Cell(220, 200, 150);  p.atm_strength = 0.3;  p.wrap = 0.12;
        p.ring = true;  p.ring_in = 1.3;  p.ring_out = 2.3;
        break;

    case PlanetType::ICE_URANUS:
        p.name = "Ice giant (Uranus)";  p.family = PlanetProfile::GAS;
        p.g1 = Cell(150, 215, 225);  p.g2 = Cell(170, 230, 235);  p.g3 = Cell(192, 240, 242);
        p.bands = 5;  p.turb = 0.1;  p.band_contrast = 0.5;
        p.atm_col = Cell(150, 230, 240);  p.atm_strength = 0.9;  p.wrap = 0.18;
        break;

    case PlanetType::ICE_NEPTUNE:
        p.name = "Ice giant (Neptune)";  p.family = PlanetProfile::GAS;
        p.g1 = Cell(30, 60, 170);  p.g2 = Cell(50, 95, 220);  p.g3 = Cell(90, 140, 245);
        p.bands = 7;  p.turb = 0.4;
        p.storms = 2;  p.storm_col = Cell(15, 25, 100);  p.storm_size = 0.12;  p.storm_alpha = 0.8;
        p.cloud_cover = 0.2;  p.cloud_alpha = 0.6;
        p.atm_col = Cell(80, 130, 255);  p.atm_strength = 1.0;  p.wrap = 0.12;
        break;

    case PlanetType::HOT_JUPITER:
        p.name = "Hot Jupiter";  p.family = PlanetProfile::GAS;
        p.g1 = Cell(25, 18, 22);  p.g2 = Cell(80, 40, 30);  p.g3 = Cell(200, 90, 40);
        p.bands = 8;  p.turb = 0.8;
        p.uniform_glow = 0.12;  p.glow_col = Cell(255, 70, 20);
        p.atm_col = Cell(255, 120, 40);  p.atm_strength = 1.0;  p.wrap = 0.3;
        break;

    case PlanetType::BROWN_DWARF:
        p.name = "Brown dwarf";  p.family = PlanetProfile::GAS;
        p.g1 = Cell(70, 20, 60);  p.g2 = Cell(150, 45, 70);  p.g3 = Cell(230, 110, 90);
        p.bands = 12;  p.turb = 1.0;
        p.storms = 3;  p.storm_col = Cell(30, 8, 25);
        p.uniform_glow = 0.3;  p.glow_col = Cell(255, 70, 60);
        p.atm_col = Cell(255, 90, 100);  p.atm_strength = 0.6;  p.wrap = 0.2;
        break;

    case PlanetType::GAS_TEAL:
        p.name = "Teal giant";  p.family = PlanetProfile::GAS;
        p.g1 = Cell(20, 110, 90);  p.g2 = Cell(60, 190, 150);  p.g3 = Cell(230, 240, 200);
        p.bands = 12;  p.turb = 0.7;
        p.storms = 2;  p.storm_col = Cell(255, 240, 120);
        p.atm_col = Cell(120, 255, 200);  p.atm_strength = 0.7;  p.wrap = 0.12;
        break;

    case PlanetType::GAS_VIOLET:
        p.name = "Violet giant";  p.family = PlanetProfile::GAS;
        p.g1 = Cell(140, 70, 130);  p.g2 = Cell(220, 130, 170);  p.g3 = Cell(250, 222, 238);
        p.bands = 11;  p.turb = 0.55;
        p.storms = 2;  p.storm_col = Cell(90, 30, 100);
        p.atm_col = Cell(240, 150, 230);  p.atm_strength = 0.6;  p.wrap = 0.12;
        break;

    default: break;
    }
    return p;
}

static inline const char* planet_type_name(PlanetType t){ return make_profile(t).name; }

static inline PlanetType random_planet_type(mt19937& rng){
    return (PlanetType)(rng() % (unsigned)PlanetType::COUNT);
}

// ============================================================
//  Планета с типом
// ============================================================

class TypedPlanet : public Planet {
    private:
        PlanetType type;
        PlanetProfile prof;
        unsigned seed;
        vector<vector<float>> glow;      // собственное свечение (лава, огни городов...)
        vector<float> ring_prof;         // радиальный профиль колец
        RGB cloud_rgb, atm_rgb, glow_rgb, ring_a_rgb, ring_b_rgb;

        void cache_colors(){
            cloud_rgb  = to_rgb(prof.cloud_col);
            atm_rgb    = to_rgb(prof.atm_col);
            glow_rgb   = to_rgb(prof.glow_col);
            ring_a_rgb = to_rgb(prof.ring_a);
            ring_b_rgb = to_rgb(prof.ring_b);
        }

        template<class T>
        double bilin(const vector<vector<T>>& m, double fx, double fy) const {
            double px = fx * x - 0.5, py = fy * y - 0.5;
            int x0 = (int)floor(px), y0 = (int)floor(py);
            double tx = px - x0, ty = py - y0;
            int x1 = (((x0 + 1) % x) + x) % x;
            x0 = ((x0 % x) + x) % x;
            int ya = min(y - 1, max(0, y0)), yb = min(y - 1, max(0, y0 + 1));
            return m[ya][x0] * (1 - tx) * (1 - ty) + m[ya][x1] * tx * (1 - ty)
                 + m[yb][x0] * (1 - tx) * ty       + m[yb][x1] * tx * ty;
        }

        // ---------------- кольца ----------------
        void build_ring_profile(){
            mt19937 rng(seed * 7919u + 13u);
            uniform_real_distribution<double> U(0.0, 1.0);
            const int n = 256;
            ring_prof.assign(n, 0.f);
            for (int oct = 0; oct < 5; oct++){
                int cnt = 6 << oct;
                vector<double> pts(cnt + 2);
                for (auto& v : pts) v = U(rng);
                double amp = pow(0.55, oct);
                for (int i = 0; i < n; i++){
                    double t = (double)i / (n - 1) * cnt;
                    int k = (int)t;
                    double f = t - k;  f = f * f * (3 - 2 * f);
                    ring_prof[i] += (float)(amp * (pts[k] * (1 - f) + pts[k + 1] * f));
                }
            }
            float mn = *min_element(ring_prof.begin(), ring_prof.end());
            float mx = *max_element(ring_prof.begin(), ring_prof.end());
            for (auto& v : ring_prof) v = (v - mn) / max(1e-6f, mx - mn);

            int gaps = 2 + (int)(U(rng) * 2);                // щели вроде деления Кассини
            for (int g = 0; g < gaps; g++){
                double c = 0.2 + 0.65 * U(rng), w = 0.02 + 0.03 * U(rng);
                for (int i = 0; i < n; i++){
                    double d = fabs((double)i / (n - 1) - c);
                    if (d < w) ring_prof[i] *= (float)sstep(w * 0.5, w, d);
                }
            }
        }

        // прозрачность кольца на доле радиуса u (0 - внутренний край, 1 - внешний); val - "тон" 0..1
        double ring_sample(double u, double& val) const {
            u = clamp01(u);
            double t = u * (ring_prof.size() - 1);
            int k = min((int)ring_prof.size() - 2, (int)t);
            double f = t - k;
            val = ring_prof[k] * (1 - f) + ring_prof[k + 1] * f;
            double edge = sstep(0.0, 0.04, u) * (1.0 - sstep(0.94, 1.0, u));
            return prof.ring_alpha * (0.15 + 0.85 * val) * edge * sstep(0.02, 0.12, val);
        }

        Vec3 spin_axis() const { return Vec3(0, cos(tilt), sin(tilt)); }

        // тень колец на поверхности
        double ring_shadow(const Vec3& P, const Vec3& l) const {
            Vec3 axis = spin_axis();
            double den = l.dot(axis);
            if (fabs(den) < 1e-4) return 1.0;
            double s = -((P - pos).dot(axis)) / den;
            if (s <= 0) return 1.0;
            Vec3 Q = P + l * s;
            double rr = (Q - pos).len() / radius;
            if (rr <= prof.ring_in || rr >= prof.ring_out) return 1.0;
            double val;
            double a = ring_sample((rr - prof.ring_in) / (prof.ring_out - prof.ring_in), val);
            return 1.0 - 0.85 * a;
        }

        // ---------------- генерация ----------------
        void fill_rocky(){
            const double PI = acos(-1.0);
            const PlanetProfile& p = prof;
            mt19937 rng(seed);
            uniform_real_distribution<double> ud(0.0, 1.0);
            auto U = [&](){ return ud(rng); };

            Perlin per = Perlin(x, y);  per.fill_sphere(7, 0.5, p.relief_freq);
            Perlin mon = Perlin(x, y);  mon.fill_sphere(5, 0.5, 2.2);
            Perlin tmp = Perlin(x, y);  tmp.fill_sphere(4, 0.5, 1.0);
            Perlin wet = Perlin(x, y);  wet.fill_sphere(4, 0.5, 1.0);
            Perlin det = Perlin(x, y);  det.fill_sphere(2, 0.6, max(4.0, x / 19.0));
            Perlin cld = Perlin(x, y);  cld.fill_sphere(6, 0.55, 2.0);
            Perlin crk = Perlin(x, y);  if (p.cracks > 0) crk.fill_sphere(4, 0.5, 3.0);
            Perlin cty = Perlin(x, y);  if (p.cities > 0) cty.fill_sphere(3, 0.5, 9.0);

            // уровень жидкости - по квантилю высот, поэтому доля воды ровно та, что в профиле
            double sea = -1.0;
            if (p.sea_frac > 0){
                vector<float> v;  v.reserve((size_t)x * y);
                for (int j = 0; j < y; j++) for (int i = 0; i < x; i++) v.push_back((float)per.get_cell(i, j));
                size_t k = min(v.size() - 1, (size_t)(p.sea_frac * v.size()));
                nth_element(v.begin(), v.begin() + k, v.end());
                sea = v[k];
            }
            double base = sea >= 0 ? sea : 0.5;

            auto elev = [&](int i, int j){
                double hh = per.get_cell(i, j);
                double rr = 1.0 - fabs(2.0 * mon.get_cell(i, j) - 1.0);
                double ld = hh - base;
                if (ld <= 0) return hh;
                return hh + p.mount * pow(rr, 4) * clamp01(ld / 0.05);
            };

            vector<double> lonc(x), lons(x);
            for (int i = 0; i < x; i++){
                double lon = ((i + 0.5) / x - 0.5) * 2 * PI;
                lonc[i] = cos(lon);  lons[i] = sin(lon);
            }

            for (int j = 0; j < y; j++){
                double lat  = (0.5 - (j + 0.5) / y) * PI;
                double pole = fabs(lat) / (PI / 2);
                double cosl = cos(lat);
                int ju = max(j - 1, 0), jd = min(j + 1, y - 1);
                double step_y = max(1, jd - ju) * (PI / y);
                double step_x = 2.0 * max(0.05, cosl) * (2 * PI / x);

                for (int i = 0; i < x; i++){
                    int il = (i - 1 + x) % x, ir = (i + 1) % x;
                    double h  = per.get_cell(i, j);
                    double d  = det.get_cell(i, j);
                    double tn = tmp.get_cell(i, j);
                    double wn = wet.get_cell(i, j);
                    double ridge = 1.0 - fabs(2.0 * mon.get_cell(i, j) - 1.0);
                    double hh = elev(i, j);
                    bool ocean = sea >= 0 && h < sea;

                    double ice_m = 0, heat = 0;
                    if (p.eyeball){
                        double ang = acos(max(-1.0, min(1.0, cosl * lonc[i]))) / PI;   // угол от подзвёздной точки
                        ice_m = sstep(0.50, 0.62, ang + (tn - 0.5) * 0.2);
                        heat  = sstep(0.34, 0.20, ang + (tn - 0.5) * 0.15);
                    } else if (p.ice_cap > 0){
                        double thr = 1.0 - 0.55 * p.ice_cap;
                        ice_m = sstep(thr - 0.05, thr + 0.05, pole + (tn - 0.5) * 0.3 + (d - 0.5) * 0.06);
                    }
                    if (heat > 0.5) ocean = false;

                    Cell col = p.mid;
                    double em = 0, light = 1.0;

                    if (ocean){
                        double depth = clamp01((sea - h) / 0.12);
                        col = ramp(p.shore, p.liquid, p.deep, depth);
                        if (p.liquid_emit > 0){
                            double crust = sstep(0.5, 0.68, wn) * 0.85;      // корка на лаве
                            col = col.mix(p.low, crust);
                            em = p.liquid_emit * (1.0 - crust) * (0.75 + 0.25 * (1.0 - depth));
                        }
                        water[j][i] = 1;
                    } else {
                        water[j][i] = 0;
                        double lt = sea >= 0 ? clamp01((hh - sea) * p.contrast)
                                             : clamp01(0.5 + (hh - 0.5) * p.contrast);
                        lt = clamp01(lt + (d - 0.5) * 0.12);
                        col = ramp(p.low, p.mid, p.high, lt);

                        if (p.alt_amount > 0)
                            col = col.mix(p.alt, sstep(0.5, 0.58, wn) * p.alt_amount);
                        if (p.ridge_amount > 0){
                            double hl = hh - base;
                            col = col.mix(p.ridge_col, sstep(0.78, 0.92, ridge) * sstep(0.0, 0.08, hl) * p.ridge_amount);
                        }
                        if (p.cracks > 0){
                            double cn = 1.0 - fabs(2.0 * crk.get_cell(i, j) - 1.0);
                            double w = 0.04 * p.cracks;
                            double line = sstep(1.0 - w * 1.3, 1.0 - w * 0.3, cn);
                            col = col.mix(p.crack_col, line * 0.9);
                            em = max(em, line * p.crack_emit);
                        }
                        if (heat > 0){
                            col = col.mix(p.hot_col, heat);
                            em = max(em, heat * p.heat_emit);
                        }
                        if (p.cities > 0){
                            double lo = 0.72 - 0.5 * p.cities;
                            double cluster = sstep(lo, lo + 0.16, cty.get_cell(i, j));
                            double spark = 0.35 + 0.65 * sstep(0.4, 0.6, d);
                            col = col.mix(p.low, cluster * 0.4);
                            em = max(em, cluster * spark * (1.0 - ice_m));
                        }
                        // лёгкая запечённая тень рельефа
                        double sx = (elev(ir, j) - elev(il, j)) / step_x;
                        double sy = (elev(i, jd) - elev(i, ju)) / step_y;
                        light = min(1.25, max(0.8, 1.0 + p.relief_shade * (sx + sy)));
                    }

                    if (ice_m > 0){
                        col = col.mix(p.ice.shade(0.94 + 0.12 * d), ice_m);
                        if (ice_m > 0.5) water[j][i] = 0;
                        em *= (1.0 - ice_m);
                    }

                    double dd = ocean ? 0.96 + d * 0.08 : 0.93 + d * 0.14;
                    cells[j][i] = col.shade(light * dd);
                    glow[j][i] = (float)clamp01(em);

                    if (p.cloud_cover > 0){
                        double lo = 0.75 - 0.6 * p.cloud_cover;
                        clouds[j][i] = sstep(lo, lo + 0.2, cld.get_cell(i, j)) * (0.85 + 0.15 * d);
                    } else clouds[j][i] = 0;
                }
            }

            // ---------- кратеры и вулканические пятна ----------
            struct Crater { double cx, cy, cz, R, cosR; int kind; };
            vector<Crater> list;
            auto add = [&](int kind, double R){
                double z = 2 * U() - 1, lo = 2 * PI * U(), s = sqrt(max(0.0, 1 - z * z));
                Crater c;
                c.cx = s * cos(lo);  c.cy = z;  c.cz = s * sin(lo);
                c.R = R;  c.cosR = cos(min(R * 1.6, 3.0));  c.kind = kind;
                list.push_back(c);
            };
            int nc = (int)(p.crater_density * 350);
            for (int k = 0; k < nc; k++){
                double R = min(0.22 * p.crater_size, 0.011 * p.crater_size / pow(U() + 0.004, 0.7));
                add(0, R);
            }
            int nv = (int)(p.vents * 60);
            for (int k = 0; k < nv; k++) add(1, 0.02 + 0.03 * U());

            if (!list.empty()){
                for (int j = 0; j < y; j++){
                    double lat = (0.5 - (j + 0.5) / y) * PI;
                    double cl = cos(lat), sl = sin(lat);
                    for (int i = 0; i < x; i++){
                        double vx = cl * lonc[i], vz = cl * lons[i];
                        double cd = 0, vm = 0, vg = 0;
                        for (const Crater& c : list){
                            double dot = vx * c.cx + sl * c.cy + vz * c.cz;
                            if (dot < c.cosR) continue;
                            double t = acos(min(1.0, dot)) / c.R;
                            if (c.kind == 0){
                                if (t < 1.0) cd += -0.28 * (1.0 - t * t) + 0.40 * sstep(0.78, 1.0, t);
                                else         cd += 0.32 * (1.0 - sstep(1.0, 1.2, t)) + 0.08 * (1.0 - sstep(1.0, 1.6, t));
                            } else if (t < 1.0){
                                vm = max(vm, 1.0 - sstep(0.45, 1.0, t));
                                vg = max(vg, 1.0 - sstep(0.0, 0.5, t));
                            }
                        }
                        if (cd != 0 && !water[j][i])
                            cells[j][i] = cells[j][i].shade(min(1.3, max(0.55, 1.0 + cd)));
                        if (vm > 0){
                            cells[j][i] = cells[j][i].mix(Cell(35, 22, 18), clamp01(vm));
                            glow[j][i] = max(glow[j][i], (float)(vg * 0.9));
                        }
                    }
                }
            }
        }

        void fill_gas(){
            const double PI = acos(-1.0);
            const PlanetProfile& p = prof;
            mt19937 rng(seed);
            uniform_real_distribution<double> ud(0.0, 1.0);
            auto U = [&](){ return ud(rng); };

            Perlin turb = Perlin(x, y);  turb.fill_sphere(5, 0.55, 2.5);
            Perlin fine = Perlin(x, y);  fine.fill_sphere(5, 0.55, max(6.0, x / 10.0));
            Perlin cld  = Perlin(x, y);  cld.fill_sphere(6, 0.55, 2.0);

            struct Storm { double lat, lon, rx; };
            vector<Storm> storms;
            for (int k = 0; k < p.storms; k++){
                Storm s;
                s.lat = (k == 0 ? -0.35 : (2 * U() - 1) * 0.7) * (PI / 2);
                s.lon = (2 * U() - 1) * PI;
                s.rx  = p.storm_size * (k == 0 ? 1.0 : 0.5 + 0.5 * U());
                storms.push_back(s);
            }

            for (int j = 0; j < y; j++){
                double lat = (0.5 - (j + 0.5) / y) * PI;
                double v = lat / (PI / 2);
                double cosl = max(0.05, cos(lat));
                for (int i = 0; i < x; i++){
                    double lon = ((i + 0.5) / x - 0.5) * 2 * PI;
                    double tb = turb.get_cell(i, j) - 0.5, fn = fine.get_cell(i, j);

                    double bp = v * p.bands * 0.5 + tb * p.turb * 2.0 + (fn - 0.5) * p.turb * 0.5;
                    double s1 = sin(2 * PI * bp), s2 = sin(2 * PI * (bp * 1.7 + 0.3 + tb * p.turb));
                    double t = clamp01(0.5 + 0.38 * s1 + 0.17 * s2);
                    t = clamp01(0.5 + (t - 0.5) * p.band_contrast);

                    Cell col = ramp(p.g1, p.g2, p.g3, t);
                    col = col.shade((0.90 + 0.2 * fn) * (1.0 - 0.2 * v * v));

                    for (const Storm& s : storms){
                        double dl = lon - s.lon;
                        while (dl >  PI) dl -= 2 * PI;
                        while (dl < -PI) dl += 2 * PI;
                        double dxs = dl * cosl / s.rx, dys = (lat - s.lat) / (s.rx * 0.55);
                        double dd = dxs * dxs + dys * dys;
                        if (dd >= 1.2) continue;
                        double spot = 1.0 - sstep(0.55, 1.0, dd);
                        double swirl = sin(7 * atan2(dys, dxs) + dd * 7) * 0.08 * spot;
                        col = col.mix(p.storm_col, spot * p.storm_alpha).shade(1.0 + swirl);
                    }

                    cells[j][i] = col;
                    water[j][i] = 0;
                    glow[j][i] = 0;
                    if (p.cloud_cover > 0){
                        double lo = 0.75 - 0.6 * p.cloud_cover;
                        clouds[j][i] = sstep(lo, lo + 0.2, cld.get_cell(i, j));
                    } else clouds[j][i] = 0;
                }
            }
        }

    protected:
        Cell shade_point(const Vec3& P, const Vec3& N, const Vec3& V,
                         const RenderContext& ctx) override {
            const double PI = acos(-1.0);

            double sin_lat, lon0;
            local_coords(N, sin_lat, lon0);
            double lat = asin(sin_lat);
            double fx, fy;
            to_map(lon0 + rot, lat, fx, fy);

            double r, g, b;
            sample_color(fx, fy, r, g, b);

            int ix = min(x - 1, (int)(fx * x));
            int iy = min(y - 1, (int)(fy * y));
            bool is_water = water[iy][ix] != 0 && prof.gloss > 0;
            double em = bilin(glow, fx, fy);

            // облака двигаются чуть быстрее поверхности
            double cfx = fmod(fx + rot * 0.12 / (2 * PI), 1.0);
            if (cfx < 0) cfx += 1.0;
            double cl = bilin(clouds, cfx, fy) * prof.cloud_alpha;
            r = r * (1 - cl) + cloud_rgb.r * cl;
            g = g * (1 - cl) + cloud_rgb.g * cl;
            b = b * (1 - cl) + cloud_rgb.b * cl;

            double lr = 0, lg = 0, lb = 0;
            double sr = 0, sg = 0, sb = 0;
            double best = -1.0;

            for (const Light& L : ctx.lights){
                if (L.owner == this) continue;

                Vec3 tl = L.pos - P;
                double dl = tl.len();
                if (dl < 1e-9) continue;
                Vec3 l = tl * (1.0 / dl);

                double diff  = N.dot(l);
                double atten = L.range > 0 ? 1.0 / (1.0 + (dl / L.range) * (dl / L.range)) : 1.0;
                best = max(best, diff * atten);

                double dn = (diff + prof.wrap) / (1.0 + prof.wrap);   // мягкий терминатор плотной атмосферы
                if (dn <= 0) continue;

                double k = L.intensity * atten * visibility(P, l, dl, L, ctx);
                if (prof.ring) k *= ring_shadow(P, l);
                if (k <= 0) continue;

                double lam = pow(dn, 0.75) * k;
                lr += L.r * lam;  lg += L.g * lam;  lb += L.b * lam;

                if (is_water && diff > 0){
                    Vec3 H = (l + V).norm();
                    double nh   = max(0.0, N.dot(H));
                    double spec = pow(nh, 90) * 0.35 * prof.gloss * (1 - cl) * k;
                    sr += L.r * spec;  sg += L.g * spec;  sb += L.b * spec;
                }
            }

            double amb = prof.ambient;
            r *= amb + 0.96 * min(1.5, lr);
            g *= amb + 0.96 * min(1.5, lg);
            b *= amb + 0.96 * min(1.5, lb);
            r += 255 * sr;  g += 255 * sg;  b += 255 * sb;

            // собственное свечение: лава, жилы, огни городов, раскалённое тело
            double night = prof.glow_night_only ? (1.0 - sstep(0.0, 0.3, best)) : 1.0;
            double e = em * night * (1.0 - 0.6 * cl);
            r += glow_rgb.r * (e + prof.uniform_glow);
            g += glow_rgb.g * (e + prof.uniform_glow);
            b += glow_rgb.b * (e + prof.uniform_glow);

            // свечение атмосферы у края диска
            double facing = clamp01(N.dot(V));
            double rim = pow(1.0 - facing, 3.0) * prof.atm_strength * clamp01(best + 0.25);
            r += atm_rgb.r * rim;  g += atm_rgb.g * rim;  b += atm_rgb.b * rim;

            return Cell((int)min(255.0, r), (int)min(255.0, g), (int)min(255.0, b));
        }

    public:
        TypedPlanet(PlanetType type, int x, int y, int dx, Drawer& draw,
                    Vec3 pos = Vec3(0, 0, 0), double radius = 1.0, unsigned seed = 1)
            : Planet(x, y, dx, draw, pos, radius),
              type(type), prof(make_profile(type)), seed(seed)
        {
            glow.assign(y, vector<float>(x, 0.f));
            cache_colors();
            if (prof.ring) build_ring_profile();
        }

        PlanetType  get_type()  const { return type; }
        const char* type_name() const { return prof.name; }

        // кольца можно добавить любой планете (или убрать)
        void set_ring(double in_r, double out_r, Cell a, Cell b, double alpha = 0.85){
            prof.ring = true;  prof.ring_in = in_r;  prof.ring_out = out_r;
            prof.ring_a = a;  prof.ring_b = b;  prof.ring_alpha = alpha;
            cache_colors();
            build_ring_profile();
        }
        void set_ring_range(double in_r, double out_r){ prof.ring_in = in_r;  prof.ring_out = out_r; }
        void remove_ring(){ prof.ring = false; }

        void fill() override {
            glow.assign(y, vector<float>(x, 0.f));
            if (prof.family == PlanetProfile::EARTH_FAMILY) Planet::fill();
            else if (prof.family == PlanetProfile::GAS)     fill_gas();
            else                                             fill_rocky();
        }

        // Кольца рисуются здесь: после того как в кадре уже все тела, с проверкой глубины
        // (кольцо закрывает то, что дальше, и скрыто за тем, что ближе).
        void draw_glow(RenderContext& ctx) override {
            if (!prof.ring) return;
            const Camera& cam = ctx.cam;
            Framebuffer& fb = ctx.fb;

            int x0, y0, x1, y1;
            double save = radius;
            radius = save * prof.ring_out;
            bool ok = screen_box(cam, x0, y0, x1, y1);
            radius = save;
            if (!ok) return;

            Vec3 axis = spin_axis();
            double num = (pos - cam.pos).dot(axis);

            for (int py = y0; py < y1; py += dx){
                for (int px = x0; px < x1; px += dx){
                    Vec3 D = cam.ray(px + dx / 2.0, py + dx / 2.0);
                    double da = D.dot(axis);
                    if (fabs(da) < 1e-7) continue;
                    double t = num / da;
                    if (t <= 0) continue;

                    Vec3 P = cam.pos + D * t;
                    double rr = (P - pos).len() / radius;
                    if (rr <= prof.ring_in || rr >= prof.ring_out) continue;

                    double val;
                    double a = ring_sample((rr - prof.ring_in) / (prof.ring_out - prof.ring_in), val);
                    if (a < 0.01) continue;

                    // освещение: тень планеты на кольце
                    double lr = 0, lg = 0, lb = 0;
                    for (const Light& L : ctx.lights){
                        if (L.owner == this) continue;
                        Vec3 tl = L.pos - P;
                        double dl = tl.len();
                        if (dl < 1e-9) continue;
                        Vec3 l = tl * (1.0 / dl);
                        double atten = L.range > 0 ? 1.0 / (1.0 + (dl / L.range) * (dl / L.range)) : 1.0;

                        Vec3 o = pos - P;
                        double along = o.dot(l), sh = 1.0;
                        if (along > 0){
                            double perp = sqrt(max(0.0, o.dot(o) - along * along));
                            sh = sstep(radius * 0.97, radius * 1.03, perp);
                        }
                        double k = L.intensity * atten * sh * (0.55 + 0.45 * fabs(axis.dot(l)));
                        lr += L.r * k;  lg += L.g * k;  lb += L.b * k;
                    }
                    double kr = 0.04 + 0.96 * min(1.4, lr);
                    double kg = 0.04 + 0.96 * min(1.4, lg);
                    double kb = 0.04 + 0.96 * min(1.4, lb);

                    double cr = (ring_a_rgb.r * (1 - val) + ring_b_rgb.r * val) * kr;
                    double cg = (ring_a_rgb.g * (1 - val) + ring_b_rgb.g * val) * kg;
                    double cb = (ring_a_rgb.b * (1 - val) + ring_b_rgb.b * val) * kb;

                    for (int yy = py; yy < py + dx; yy++){
                        for (int xx = px; xx < px + dx; xx++){
                            if (!fb.inside(xx, yy)) continue;
                            size_t i = (size_t)yy * fb.w + xx;
                            if (t >= fb.z[i]) continue;           // перед кольцом что-то стоит
                            fb.set(xx, yy,
                                   min(255.0, fb.col[3 * i]     * (1 - a) + cr * a),
                                   min(255.0, fb.col[3 * i + 1] * (1 - a) + cg * a),
                                   min(255.0, fb.col[3 * i + 2] * (1 - a) + cb * a));
                        }
                    }
                }
            }
        }
};
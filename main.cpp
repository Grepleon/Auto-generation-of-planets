#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <vector>

int main() {
    const int width = 200;
    const int height = 100;
    const int channels = 3; // RGB

    // Белое изображение: все байты = 255
    std::vector<unsigned char> img(width * height * channels, 255);

    // i - строка (y), j - столбец (x)
    auto setBlack = [&](int i, int j) {
        if (i < 0 || i >= height || j < 0 || j >= width) return;
        unsigned char* p = &img[(i * width + j) * channels];
        p[0] = p[1] = p[2] = 0;
    };

    // Примеры
    setBlack(50, 100);                       // один пиксель
    for (int j = 20; j < 180; ++j)           // линия
        setBlack(30, j);
    for (int i = 60; i < 80; ++i)            // квадрат
        for (int j = 10; j < 30; ++j)
            setBlack(i, j);
    for (int k = 0; k < height; ++k)         // диагональ
        setBlack(k, k);

    // Сохранение в PNG
    stbi_write_png("dats/result.png", width, height, channels, img.data(), width * channels);
}
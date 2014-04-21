#include <Magick++.h>
#include <stdexcept>
#include "bootstrap.h"



void initImageFromFile(Image *img, const std::string &path)
{
    Magick::Image mi;
    mi.read(path);

    Magick::Geometry size = mi.size();
    img->resize(size.width(), size.height());

    Magick::PixelPacket *pixels = mi.getPixels(0, 0, img->width, img->height);

    for (size_t y = 0; y < img->height; ++y) {
        for (size_t x = 0; x < img->width; ++x) {
            Magick::PixelPacket *pixel = pixels + y * img->width + x;
            img->values[y][x].r = pixel->red;
            img->values[y][x].g = pixel->green;
            img->values[y][x].b = pixel->blue;
            img->values[y][x].a = pixel->opacity;
        }
    }
}



void dumpImage(const Image &img, const std::string &path)
{
    Magick::Geometry size(img.width, img.height, 0, 0);
    Magick::Image mi(size, Magick::Color(255, 255, 255));

    mi.modifyImage();
    mi.type(Magick::TrueColorType);

    Magick::PixelPacket *pixels = mi.getPixels(0, 0, img.width, img.height);

    for (size_t y = 0; y < img.height; ++y) {
        for (size_t x = 0; x < img.width; ++x) {
            Magick::PixelPacket *pixel = pixels + y * img.width + x;
            *pixel = Magick::Color(img.values[y][x].r, img.values[y][x].g,
                                   img.values[y][x].b, img.values[y][x].a);
        }
    }

    mi.syncPixels();
    mi.write(path);
}

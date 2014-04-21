#include "../convolution.h"


static uint8_t truncate(double v)
{
    int vi = static_cast<int>(v);
    if (vi < 0)   vi = 0;
    if (vi > 255) vi = 255;

    return static_cast<uint8_t>(vi);
}

// Serial implementation
// Adapted from http://lodev.org/cgtutor/filtering.html
void convolution(Image *out, const Image &in, const Filter &filter)
{
    out->resize(in.width, in.height);

    for (size_t y = 0; y < in.height; ++y) {
        for (size_t x = 0; x < in.width; ++x) {
            double red = 0, green = 0, blue = 0;

            for (size_t filterY = 0; filterY < filter.height; ++filterY) {
                for (size_t filterX = 0; filterX < filter.width; ++filterX) {
                    size_t imageX = (x - filter.width / 2 + filterX + in.width) % in.width;
                    size_t imageY = (y - filter.height / 2 + filterY + in.height) % in.height;

                    red   += in.values[imageY][imageX].r * filter.values[filterY][filterX];
                    green += in.values[imageY][imageX].g * filter.values[filterY][filterX];
                    blue  += in.values[imageY][imageX].b * filter.values[filterY][filterX];
                }
            }
            // truncate values smaller than 0 and larger than 255
            out->values[y][x].r = truncate(red);
            out->values[y][x].g = truncate(green);
            out->values[y][x].b = truncate(blue);
            out->values[y][x].a = in.values[y][x].a;
        }
    }
}

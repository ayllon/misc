#include "bootstrap.h"
#include <fstream>
#include <stdexcept>


void initFilterFromFile(Filter *filter, const std::string &path)
{
    std::ifstream in(path);
    if (in.fail())
        throw std::runtime_error("Could not open " + path);

    size_t w, h;
    in >> w >> h;
    filter->resize(w, h);

    for (size_t y = 0; y < h; ++y)
        for (size_t x = 0; x < w; ++x)
            in >> filter->values[y][x];
}

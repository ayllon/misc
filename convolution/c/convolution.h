#ifndef _CONVOLUTION_H_
#define _CONVOLUTION_H_

#include <iostream>
#include <stdint.h>

// Matrix representation
template <class T>
class Matrix
{
public:
    // Internal representation
    size_t   width, height;
    T **values;

    // Default constructor
    Matrix(): width(0), height(0), values(NULL)
    {
    };

    // Initializes the matrix from the given width and height
    Matrix(size_t w, size_t h): width(w), height(h), values(NULL)
    {
        _allocate();
    }

    // Initializes a square matrix
    explicit Matrix(size_t s): width(s), height(s), values(NULL)
    {
        _allocate();
    }

    // Copy constructor
    Matrix(const Matrix<T> &src): width(src.width), height(src.height), values(NULL)
    {
        _allocate();
        _copy(src);
    }

    // Destructor
    ~Matrix()
    {
        _free();
    }

    // Assign
    Matrix<T>& operator = (const Matrix<T> &src)
    {
        _free();
        width  = src.width;
        height = src.height;
        _allocate();
        _copy(src);
        return *this;
    }

    // Resize
    void resize(size_t w, size_t h)
    {
        _free();
        width  = w;
        height = h;
        _allocate();
    }

protected:
    void _allocate(void)
    {
        values = new T*[height];
        for (size_t y = 0; y < height; ++y)
            values[y] = new T[width];
    }

    void _copy(const Matrix<T>& src)
    {
        for (size_t y = 0; y < height; ++y)
            for (size_t x = 0; x < width; ++x)
                values[y][x] = src.values[y][x];
    }

    void _free()
    {
        for (size_t y = 0; y < height; ++y)
            delete [] values[y];
        delete [] values;
        values = NULL;
        width = height = 0;
    }
};

// Stream operator
template <class T>
std::ostream& operator << (std::ostream &out, const Matrix<T> &m)
{
    for (size_t y = 0; y < m.height; ++y) {
        for (size_t x = 0; x < m.width; ++x)
            out << m.values[y][x] << " ";
        out << std::endl;
    }
    return out;
}

// Pixel
struct Pixel
{
    uint8_t r, g, b, a;

    Pixel(): r(0), g(0), b(0), a(1) {};
};

// Typedefs
typedef Matrix<double> Filter;
typedef Matrix<Pixel> Image;

// Filter
void convolution(Image *out, const Image &in, const Filter &filter);

#endif // _CONVOLUTION_H

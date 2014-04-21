#include <ctime>
#include <boost/timer.hpp>
#include <fcntl.h>
#include <iostream>
#include <iomanip>
#include <Magick++.h>
#include <unistd.h>
#include "bootstrap.h"
#include "../convolution.h"



int usage(const char* bin)
{
    std::cerr << "Usage: " << bin << " [picture] [filter] [output bitmap]" << std::endl;
    return 1;
}


// elapsed in milliseconds
static double getElapsed(struct timespec &end, struct timespec &start)
{
    double startns = start.tv_sec * 1000000000LL + start.tv_nsec;
    double endns   = end.tv_sec * 1000000000LL + end.tv_nsec;
    return (endns - startns) / 1000000;
}



int main(int argc, const char *argv[])
{
    if (argc < 4)
        return usage(argv[0]);

    // This is required
    Magick::InitializeMagick(argv[0]);

    const char *imagePath  = argv[1];
    const char *filterPath = argv[2];
    const char *outputPath = argv[3];

    std::cout << "Image:  " << imagePath << std::endl
              << "Filter: " << filterPath << std::endl;

    try {
        // Load the filter
        Filter filter;
        initFilterFromFile(&filter, filterPath);
        std::cout << std::fixed << filter << std::endl;

        // Load the image
        Image image;
        initImageFromFile(&image, imagePath);

        // Process
        struct timespec start;
        clock_gettime(CLOCK_MONOTONIC_RAW, &start);

        Image output;
        convolution(&output, image, filter);

        struct timespec end;
        clock_gettime(CLOCK_MONOTONIC_RAW, &end);

        // Dump
        dumpImage(output, outputPath);

        // Print time used
        double elapsed = getElapsed(end, start);
        std::cout << "Took " << elapsed << " milliseconds"
                  << std::endl
                  << "(" << elapsed / 1000 << " seconds)"
                  << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    return 0;
}

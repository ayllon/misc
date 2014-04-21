package main

import (
	"fmt"
	"image/color"
	"runtime"
)


func truncate(v float64) uint8 {
    vi := int(v)
    if vi < 0 {
    	vi = 0;
    }
    if vi > 255 {
    	vi = 255;
    }
    return uint8(vi)
}


func convolutionPartial(id int, channel chan int, out [][]color.RGBA, in [][]color.RGBA,
                        filter [][]float64, start int, length int,
                        height int, width int) {
	filterHeight := len(filter)
	filterWidth  := 0
	if filterHeight > 0 {
		filterWidth = len(filter[0])
	}
	
	end := start + length
    for y := start; y < end; y++ {
    	outRow := out[y]
        for x := 0; x < width; x++ {
            var red, green, blue float64

            for filterY := 0; filterY < filterHeight; filterY++ {
                for filterX := 0; filterX < filterWidth; filterX++ {
                    imageX := (x - filterWidth / 2 + filterX + width) % width;
                    imageY := (y - filterHeight / 2 + filterY + height) % height;
                    
                    filterCell := filter[filterY][filterX]
                    inCell     := in[imageY][imageX]

                    red   += float64(inCell.R) * filterCell
                    green += float64(inCell.G) * filterCell
                    blue  += float64(inCell.B) * filterCell
                }
            }
            // truncate values smaller than 0 and larger than 255
            outRow[x].R = truncate(red);
            outRow[x].G = truncate(green);
            outRow[x].B = truncate(blue);
            outRow[x].A = in[y][x].A;
        }
    }
	
	// Done
	channel <- id
}

func Convolution(in [][]color.RGBA, filter [][]float64) (out [][]color.RGBA) {
	// Sizes
	height := len(in)
	width  := 0
	if height > 0 {
		width = len(in[0])
	}
	
	// Init the output image	
	out = make([][]color.RGBA, height)
	for y := 0; y < height; y++ {
		out[y] = make([]color.RGBA, width)
	}
	
	// Apply the filter in parallel
	nRoutines       := runtime.GOMAXPROCS(-1)
	nrowsPerRoutine := height / nRoutines
	spareRows       := height % nrowsPerRoutine
	
	channel := make(chan int, nRoutines)
	for chunk := 0; chunk < nRoutines - 1; chunk ++ {
		go convolutionPartial(chunk, channel, out, in, filter,
		                      chunk * nrowsPerRoutine,
		                      nrowsPerRoutine,
		                      height, width)
	}
	
	convolutionPartial(nRoutines - 1, channel, out, in, filter,
	                   height - spareRows - nrowsPerRoutine,
	                   spareRows + nrowsPerRoutine,
	                   height, width)
	
	// Wait
	for i := 0; i < nRoutines; i++ {
		id := <- channel
		fmt.Println("Finished", id);
	}
	
	return
}

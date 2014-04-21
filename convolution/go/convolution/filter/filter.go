package filter

import (
	"fmt"
	"io"
	"os"
)


func readDimensions(file io.Reader) (width int, height int) {
	fmt.Fscanf(file, "%d %d", &width, &height)
	return
}


func Load(path string) (filter [][]float64, err error) {
	// Open file
	file, err := os.Open(path)
	if err != nil {
		return
	}
	defer file.Close()
	
	// Read dimensions
	width, height := readDimensions(file)

	// Init the matrix
	filter = make([][]float64, height)
	
	for y, _:= range filter {
		filter[y] = make([]float64, width)
		for x, _ := range filter[y] {
			fmt.Fscanf(file, "%f", &filter[y][x])
		}
	}
	
	return
}

package main

import (
	"fmt"
	"os"
	"time"
	"./filter"
	"./image"
)


func usage(bin string) {
	fmt.Println("Usage:", bin, "[picture] [filter] [output bitmap]")
}


func main() {
	if len(os.Args) < 4 {
		usage(os.Args[0])
	}
	
	imagePath  := os.Args[1]
	filterPath := os.Args[2]
	outputPath := os.Args[3]
	
	fmt.Println("Image: ", imagePath)
	fmt.Println("Filter:", filterPath)
	
	// Load the filter
	filterArray, err := filter.Load(filterPath)
	if err != nil {
		fmt.Println("Could not load the filter:", err)
		return
	}
	fmt.Println(filterArray)
	
	// Load the image
	imageArray, err := image.Load(imagePath)
	if err != nil {
		fmt.Println("Could not load the image:", err)
		return
	}
	
	// Process
	start := time.Now()
	outArray := Convolution(imageArray, filterArray)
	elapsed := time.Since(start)
	
	// Dump
	err = image.Save(outArray, outputPath)
	if err != nil {
		fmt.Println("Could not save the image:", err)
	}
	
	fmt.Println("Took", elapsed.Seconds() * 1000, "milliseconds")
	fmt.Println("(", elapsed.Seconds(), "seconds)")
}

package image

import (
	"image"
	"image/color"
	"image/png"
	"os"
)


func Load(path string) (imgArray [][]color.RGBA, err error) {
	file, err := os.Open(path)
	if err != nil {
		return
	}
	defer file.Close()
	
	img, _, err := image.Decode(file)
	if err != nil {
		return
	}
	
	width  := img.Bounds().Max.X
	height := img.Bounds().Max.Y
	
	imgArray = make([][]color.RGBA, height)
	for y := 0; y < height; y++ {
		imgArray[y] = make([]color.RGBA, width)
		for x := 0; x < width; x++ {
			imgArray[y][x] = img.At(x, y).(color.RGBA)
		}
	}

	return
}


func Save(imgArray [][]color.RGBA, path string) (err error) {
	height := len(imgArray)
	width  := 0
	if height > 0 {
		width = len(imgArray[0])
	}
	
	bounds := image.Rect(0, 0, width, height)
	img := image.NewRGBA(bounds)
	
	for y := 0; y < height; y++ {
		for x := 0; x < width; x++ {
			img.Set(x, y, imgArray[y][x])
		}
	}
	
	file, err := os.Create(path)
	if err != nil {
		return
	}
	defer file.Close()
	
	err = png.Encode(file, img)
	return
}

#include "BMPImage.h"
#include "Pixel.h"
#include <iostream>

int main() {
	std::cout << "Image from File :" << std::endl;
	BMPImage image("Images/bmp_24.bmp");
	std::cout << image << std::endl;
	image.resize(100,200);
	image.multiplySize(-0.5);
	std::cout << image << std::endl;
	image.save("Images/reverse_bmp_24_v2");


	std::cout << std::endl;


	std::cout << "======================================" << std::endl;
	std::cout << "New Image :" << std::endl;
	BMPImage newImage(2,2);
	std::cout << newImage << std::endl;
	newImage.setPixel(0, 0, 255, 255, 255);
	newImage.setPixel(0, 1, 255, 0, 0);
	newImage.setPixel(1, 0, 0, 255, 0);
	newImage.setPixel(1, 1, 0, 0, 255);
	newImage.setResolution(30000, 30000);
	newImage.save("Images/100_100_image");
	std::cout << std::endl;

	std::cout << "======================================" << std::endl;
	std::cout << "Deep Color Image :" << std::endl;
	BMPImage deepImage(BMPImage::DEEP_COLOR_BIT_SIZE);
	deepImage.resize(2, 2);
	std::cout << deepImage << std::endl;
	deepImage.setPixel(0, 0, 255, 255, 255, 100);
	deepImage.setPixel(0, 1, 255, 0, 0, 100);
	deepImage.setPixel(1, 0, 0, 255, 0, 255);
	deepImage.setPixel(1, 1, 0, 0, 255, 255);
	deepImage.multiplySize(50);
	std::cout << deepImage << std::endl;

	deepImage.multiplySize(0.5);
	std::cout << deepImage << std::endl;

	deepImage.save("Images/deep_image");
	std::cout << std::endl;

} 


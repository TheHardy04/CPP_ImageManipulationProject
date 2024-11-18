#pragma once
#include <memory>
#include <iostream>

class Pixel
{
	uint16_t size;
	std::unique_ptr<uint8_t[]> pixel_data;
public:

	// Constants
	static constexpr uint16_t DEEP_COLOR_BYTE_SIZE = 4;
	static constexpr uint16_t TRUE_COLOR_BYTE_SIZE = 3;
	static constexpr uint16_t MONOCHROME_BYTE_SIZE = 1;


	Pixel(uint16_t size = TRUE_COLOR_BYTE_SIZE);
	Pixel(uint16_t size, std::unique_ptr<uint8_t[]> data);
	Pixel(uint16_t size,uint8_t* data);
	Pixel(uint8_t red, uint8_t green, uint8_t blue);
	Pixel(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);

	Pixel(const Pixel& pixel);
	~Pixel();
	Pixel& operator=(const Pixel& other);

	uint16_t getSize() const;
	uint8_t getRed() const;
	uint8_t getGreen() const;
	uint8_t getBlue() const;
	uint8_t getAlpha() const;
	
	friend std::ostream& operator<<(std::ostream& os, const Pixel& pixel);

};


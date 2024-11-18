#include <cstdint>
#include <memory>
#include <iostream>
#include "Pixel.h"

/// <summary>
/// Default constructor
/// </summary>
/// <param name="size"></param>
Pixel::Pixel(uint16_t size) : size(size), pixel_data(std::make_unique<uint8_t[]>(size))
{
}

/// <summary>
/// Move constructor
/// </summary>
/// <param name="size"></param>
/// <param name="data"></param>
Pixel::Pixel(uint16_t size, std::unique_ptr<uint8_t[]> data) : size(size), pixel_data(std::move(data))
{
}

/// <summary>
/// Constructor for raw data
/// </summary>
/// <param name="size"></param>
/// <param name="data"></param>
Pixel::Pixel(uint16_t size, uint8_t* data) : size(size), pixel_data(std::make_unique<uint8_t[]>(size))
{
	std::copy(data, data + size, pixel_data.get());
}

/// <summary>
/// RGB constructor
/// </summary>
/// <param name="red"></param>
/// <param name="green"></param>
/// <param name="blue"></param>
Pixel::Pixel(uint8_t red, uint8_t green, uint8_t blue) :
	size(TRUE_COLOR_BYTE_SIZE),
	pixel_data(std::make_unique<uint8_t[]>(TRUE_COLOR_BYTE_SIZE))
{
	pixel_data[0] = red;
	pixel_data[1] = green;
	pixel_data[2] = blue;
}

/// <summary>
/// RBGA constructor
/// </summary>
/// <param name="red"></param>
/// <param name="green"></param>
/// <param name="blue"></param>
/// <param name="alpha"></param>
Pixel::Pixel(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha) :
	size(DEEP_COLOR_BYTE_SIZE),
	pixel_data(std::make_unique<uint8_t[]>(DEEP_COLOR_BYTE_SIZE))
{
	pixel_data[0] = red;
	pixel_data[1] = green;
	pixel_data[2] = blue;
	pixel_data[3] = alpha;
}

// Copy constructor
Pixel::Pixel(const Pixel& pixel) : size(pixel.size), pixel_data(std::make_unique<uint8_t[]>(pixel.size))
{
	if (pixel.size == 0) return;
	if(pixel.size != size)
	{
		size = pixel.size;
	}
	std::copy(pixel.pixel_data.get(), pixel.pixel_data.get() + pixel.size, pixel_data.get());
}

// Destructor 
Pixel::~Pixel() = default;

Pixel& Pixel::operator=(const Pixel& other)
{
	if (this == &other)
	{
		return *this;
	}
	if (other.size == 0)
	{
		size = 0;
		pixel_data = nullptr;
		return *this;
	}
	if (other.size != size)
	{
		size = other.size;
		pixel_data = std::make_unique<uint8_t[]>(size);
	}
	pixel_data = std::make_unique<uint8_t[]>(other.size);
	std::copy(other.pixel_data.get(), other.pixel_data.get() + other.size, pixel_data.get());
	return *this;
}

uint16_t Pixel::getSize() const
{
	return size;
}

uint8_t Pixel::getRed() const
{
	if (size == DEEP_COLOR_BYTE_SIZE || size == TRUE_COLOR_BYTE_SIZE) return pixel_data[0];
	throw std::runtime_error("Invalid Byte Size to get red component");
}

uint8_t Pixel::getGreen() const
{
	if (size == DEEP_COLOR_BYTE_SIZE || size == TRUE_COLOR_BYTE_SIZE) return pixel_data[1];
	throw std::runtime_error("Invalid Byte Size to get green component");
}

uint8_t Pixel::getBlue() const
{
	if (size == DEEP_COLOR_BYTE_SIZE || size == TRUE_COLOR_BYTE_SIZE) return pixel_data[2];
	throw std::runtime_error("Invalid Byte Size to get blue component");
}

uint8_t Pixel::getAlpha() const
{
	if (size == DEEP_COLOR_BYTE_SIZE) return pixel_data[3];
	throw std::runtime_error("Invalid Byte Size to get alpha component");
}

std::ostream& operator<<(std::ostream& os, const Pixel& pixel)
{
	for (int i = 0; i < pixel.size; i++)
	{
		os << static_cast<int>(pixel.pixel_data[i]) << " ";
	}
	return os;
}

#include <iostream>
#include <fstream>
#include <algorithm>
#include <memory>
#include <cstdlib>


#include "BMPImage.h"

// <summary>
/// Convert a value to little-endian (or big-endian) by swapping bytes.
/// </summary>
/// <param name="value"> The value to convert </param>
/// <returns>Converted value in little-endian or big-endian order</returns>
template <typename T>
T ConvertEndian(T value)
{
	static_assert(std::is_integral<T>::value, "T must be an integral type."); // check that the value is an integer
	T result = 0;
	int numBytes = sizeof(T);

	for (int i = 0; i < numBytes; i++)
	{
		result |= ((value >> (i * 8)) & 0xFF) << ((numBytes - 1 - i) * 8);
	}

	return result;
}

/// <summary>
///  Default constructor
/// </summary>
/// <param name="bitCount">Color Depth</param>
BMPImage::BMPImage(uint16_t bitCount)
{
	_infoHeader = BMPInfoHeader{
		BM_INFO_HEADER_SIZE, 0, 0, COLOR_PLANES_NUMBER, bitCount, BI_RGB, 0, BM_DEFAULT_RESOLUTION,
		BM_DEFAULT_RESOLUTION, 0, 0
	};
	_fileHeader = BMPFileHeader{
		BM_SIGNATURE, BM_FILE_HEADER_SIZE + BM_INFO_HEADER_SIZE, 0, 0, BM_FILE_HEADER_SIZE + BM_INFO_HEADER_SIZE
	};


}

/// <summary>
/// Constructor with width and height
/// </summary>
/// <param name="width"></param>
/// <param name="height"></param>
/// <param name="bitCount">Color Depth</param>
BMPImage::BMPImage(int32_t width, int32_t height, uint16_t bitCount)
{
	_infoHeader = BMPInfoHeader{
		BM_INFO_HEADER_SIZE, width, height, COLOR_PLANES_NUMBER, bitCount, BI_RGB, 0, BM_DEFAULT_RESOLUTION,
		BM_DEFAULT_RESOLUTION, 0, 0
	};
	_fileHeader = BMPFileHeader{
		BM_SIGNATURE, BM_FILE_HEADER_SIZE + BM_INFO_HEADER_SIZE, 0, 0, BM_FILE_HEADER_SIZE + BM_INFO_HEADER_SIZE
	};
	_updateHeaders();
	_pixelData.resize(width * height);
}

/// <summary>
/// load an image from a file
/// </summary>
/// <param name="filename"></param>
BMPImage::BMPImage(const char* filename)
{
	load(filename);
}

/// <summary>
/// Destroy the image object
/// </summary>
BMPImage::~BMPImage() = default;

uint16_t BMPImage::_getByteCount() const
{
	if (_infoHeader.bitCount == DEEP_COLOR_BIT_SIZE)
		return 4;
	if (_infoHeader.bitCount == TRUE_COLOR_BIT_SIZE)
		return 3;
	if (_infoHeader.bitCount == GRAY_SCALE_BIT_SIZE)
		return 4;
	if (_infoHeader.bitCount == MONOCHROME_BIT_SIZE)
		return 1;
	throw std::runtime_error("Invalid bit count");
}

void BMPImage::_readHeaders(std::ifstream& file)
{
	// Read the file header
	// Read the file type
	file.read(reinterpret_cast<char*>(&_fileHeader.fileType), sizeof(_fileHeader.fileType));

	// Check if it's a BMP file by looking for the "BM" signature
	if (_fileHeader.fileType != BM_SIGNATURE)
	{
		throw std::runtime_error("File is not a BMP file.");
	}

	// Read the file size
	file.read(reinterpret_cast<char*>(&_fileHeader.fileSize), sizeof(_fileHeader.fileSize));
	// Read the reserved fields
	file.read(reinterpret_cast<char*>(&_fileHeader.reserved1), sizeof(_fileHeader.reserved1));
	file.read(reinterpret_cast<char*>(&_fileHeader.reserved2), sizeof(_fileHeader.reserved2));
	// Read the offset to the pixel data
	file.read(reinterpret_cast<char*>(&_fileHeader.offsetData), sizeof(_fileHeader.offsetData));

	// Read the info header
	// Read the size of the info header
	file.read(reinterpret_cast<char*>(&_infoHeader.size), sizeof(_infoHeader.size));
	// Read the width and height of the image
	file.read(reinterpret_cast<char*>(&_infoHeader.width), sizeof(_infoHeader.width));
	file.read(reinterpret_cast<char*>(&_infoHeader.height), sizeof(_infoHeader.height));
	// Read the number of color planes
	file.read(reinterpret_cast<char*>(&_infoHeader.planes), sizeof(_infoHeader.planes));
	// Read the number of bits per pixel
	file.read(reinterpret_cast<char*>(&_infoHeader.bitCount), sizeof(_infoHeader.bitCount));
	// Read the compression method
	file.read(reinterpret_cast<char*>(&_infoHeader.compression), sizeof(_infoHeader.compression));
	// Read the size of the raw bitmap data
	file.read(reinterpret_cast<char*>(&_infoHeader.sizeImage), sizeof(_infoHeader.sizeImage));
	// Read the horizontal and vertical resolution
	file.read(reinterpret_cast<char*>(&_infoHeader.xPixelsPerMeter), sizeof(_infoHeader.xPixelsPerMeter));
	file.read(reinterpret_cast<char*>(&_infoHeader.yPixelsPerMeter), sizeof(_infoHeader.yPixelsPerMeter));
	// Read the number of colors in the color palette
	file.read(reinterpret_cast<char*>(&_infoHeader.colorsUsed), sizeof(_infoHeader.colorsUsed));
	// Read the number of important colors
	file.read(reinterpret_cast<char*>(&_infoHeader.colorsImportant), sizeof(_infoHeader.colorsImportant));

	// Check if the image is uncompressed
	if (_infoHeader.compression != BI_RGB)
	{
		throw std::runtime_error("Image is compressed.");
	}

	// Check if the image bit count is valid
	if (_infoHeader.bitCount != TRUE_COLOR_BIT_SIZE && _infoHeader.bitCount != DEEP_COLOR_BIT_SIZE)
	{
		throw std::runtime_error("Image bit count is not valid.");
	}
}

void BMPImage::_readPixels(std::ifstream& file)
{
	// Calculate the number of bytes per row
	int bytesPerRow = ((_infoHeader.width * _infoHeader.bitCount + 31) / 32) * 4;

	// Pixel data
	for (int i = 0; i < _infoHeader.height; i++)
	{
		for (int j = 0; j < _infoHeader.width; j++)
		{
			std::uint16_t pixelSize = _getByteCount();
			auto pixel_ptr = std::make_unique<uint8_t[]>(pixelSize);
			file.read(reinterpret_cast<char*>(pixel_ptr.get()), pixelSize);
			// change the order of the pixel data
			std::swap(pixel_ptr[0], pixel_ptr[2]); // Swap red and blue channels
			_pixelData.push_back(Pixel(pixelSize, pixel_ptr.get()));
		}
	}
}

void BMPImage::_writeHeaders(std::ofstream& file) const
{
	//Write the file header
	file.write(reinterpret_cast<const char*>(&_fileHeader.fileType), sizeof(_fileHeader.fileType));
	file.write(reinterpret_cast<const char*>(&_fileHeader.fileSize), sizeof(_fileHeader.fileSize));
	file.write(reinterpret_cast<const char*>(&_fileHeader.reserved1), sizeof(_fileHeader.reserved1));
	file.write(reinterpret_cast<const char*>(&_fileHeader.reserved2), sizeof(_fileHeader.reserved2));
	file.write(reinterpret_cast<const char*>(&_fileHeader.offsetData), sizeof(_fileHeader.offsetData));

	file.write(reinterpret_cast<const char*>(&_infoHeader.size), sizeof(_infoHeader.size));
	file.write(reinterpret_cast<const char*>(&_infoHeader.width), sizeof(_infoHeader.width));
	file.write(reinterpret_cast<const char*>(&_infoHeader.height), sizeof(_infoHeader.height));
	file.write(reinterpret_cast<const char*>(&_infoHeader.planes), sizeof(_infoHeader.planes));
	file.write(reinterpret_cast<const char*>(&_infoHeader.bitCount), sizeof(_infoHeader.bitCount));
	file.write(reinterpret_cast<const char*>(&_infoHeader.compression), sizeof(_infoHeader.compression));
	file.write(reinterpret_cast<const char*>(&_infoHeader.sizeImage), sizeof(_infoHeader.sizeImage));
	file.write(reinterpret_cast<const char*>(&_infoHeader.xPixelsPerMeter), sizeof(_infoHeader.xPixelsPerMeter));
	file.write(reinterpret_cast<const char*>(&_infoHeader.yPixelsPerMeter), sizeof(_infoHeader.yPixelsPerMeter));
	file.write(reinterpret_cast<const char*>(&_infoHeader.colorsUsed), sizeof(_infoHeader.colorsUsed));
	file.write(reinterpret_cast<const char*>(&_infoHeader.colorsImportant), sizeof(_infoHeader.colorsImportant));
}

void BMPImage::_writePixels(std::ofstream& file) const
{
	// Write the pixel data
	for (int i = 0; i < _infoHeader.height; i++)
	{
		for (int j = 0; j < _infoHeader.width; j++)
		{
			Pixel pixel = _pixelData[i * _infoHeader.width + j];
			std::streamsize pixelSize = _getByteCount();
			auto pixel_ptr = std::make_unique<uint8_t[]>(pixelSize);
			pixel_ptr[0] = pixel.getRed();
			pixel_ptr[1] = pixel.getGreen();
			pixel_ptr[2] = pixel.getBlue();
			if (_infoHeader.bitCount == DEEP_COLOR_BIT_SIZE)
			{
				pixel_ptr[3] = pixel.getAlpha();
			}
			std::swap(pixel_ptr[0], pixel_ptr[2]); // Swap red and blue channels
			file.write(reinterpret_cast<const char*>(pixel_ptr.get()), pixelSize);
		}
		// Add padding if needed
		int paddingSize = (4 - (_infoHeader.width * _getByteCount()) % 4) % 4;
		uint8_t paddingData[4] = {0, 0, 0, 0};
		file.write(reinterpret_cast<const char*>(paddingData), paddingSize);
	}
}

void BMPImage::_updateHeaders()
{
	_infoHeader.sizeImage = _infoHeader.width * _infoHeader.height * _getByteCount();
	_fileHeader.fileSize = BM_FILE_HEADER_SIZE + BM_INFO_HEADER_SIZE + _infoHeader.sizeImage;
	_fileHeader.offsetData = BM_FILE_HEADER_SIZE + BM_INFO_HEADER_SIZE;
}

void BMPImage::_resizePixelsData(int32_t newWidth, int32_t newHeight)
{
	std::vector<Pixel> newPixelData(newHeight * newWidth);
	const int32_t minHeight = std::min(_infoHeader.height, newHeight);
	const int32_t minWidth = std::min(_infoHeader.width, newWidth);
	const int32_t maxHeight = std::max(_infoHeader.height, newHeight);
	const int32_t maxWidth = std::max(_infoHeader.width, newWidth);

	for (int i = 0; i < maxHeight; i++)
	{
		for (int j = 0; j < maxWidth; j++)
		{
			if (i < minHeight && j < minWidth)
			{
				newPixelData[i * newWidth + j] = _pixelData[i * _infoHeader.width + j];
			}
		}
	}

	// update the headers
	_infoHeader.height = newHeight;
	_infoHeader.width = newWidth;
	// resize the pixel data
	_pixelData.resize(newHeight * newWidth);
	// copy the data of the new pixel data to the pixel data
	Pixel* newPixelDataPtr = newPixelData.data();
	std::copy(newPixelDataPtr, newPixelDataPtr + newHeight * newWidth, _pixelData.data());

	_updateHeaders();
}

/// <summary>
/// Open the image using the default OS image viewer
/// </summary>
/// <param name="filename"></param>
void BMPImage::openImage(const std::string& filename)
{
    #if defined(_WIN32) || defined(_WIN64)
        std::string command = "start " + filename;   // Windows
    #elif defined(__APPLE__)
        std::string command = "open " + filename;    // macOS
    #elif defined(__linux__)
        std::string command = "xdg-open " + filename; // Linux
    #else
        #error "Unsupported OS"
    #endif
    system(command.c_str());
}

/// <summary>
/// load a BMP image from a file
/// </summary>
/// <param name="filename"></param>
void BMPImage::load(const char* filename)
{
	std::ifstream file(filename, std::ios::binary);
	if (!file)
	{
		throw std::runtime_error("Could not open file");
	}

	_readHeaders(file);

	_readPixels(file);

	file.close();
	std::cout << "Image loaded successfully" << std::endl;
}

/// <summary>
/// save the image into a file
/// </summary>
/// <param name="filename"></param>
void BMPImage::save(const char* filename) const
{
	std::string fileStr(filename);
	if (fileStr.find(".bmp") == std::string::npos)
	{
		fileStr += ".bmp";
	}

	std::ofstream file(fileStr, std::ios::binary);
	if (!file)
	{
		throw std::runtime_error("Could not open file");
	}

	_writeHeaders(file);

	_writePixels(file);

	file.close();
	std::cout << "Image saved successfully" << std::endl;

	openImage(fileStr);

}




uint32_t BMPImage::getWidth() const
{
	return _infoHeader.width;
}

uint32_t BMPImage::getHeight() const
{
	return _infoHeader.height;
}

/// <summary>
/// Get the pixel at the specified row and column
/// </summary>
/// <param name="x">row</param>
/// <param name="y">column</param>
/// <returns></returns>
Pixel BMPImage::getPixel(const uint16_t x, const uint16_t y) const
{
	if (x >= _infoHeader.width || y >= _infoHeader.height)
	{
		throw std::out_of_range("Pixel coordinates are out of bounds");
	}
	Pixel pixel = _pixelData[y * _infoHeader.width + x];
	return pixel;
}

/// <summary>
/// Set the pixel at the specified row and column
/// </summary>
/// <param name="x"></param>
/// <param name="y"></param>
/// <param name="r"></param>
/// <param name="g"></param>
/// <param name="b"></param>
void BMPImage::setPixel(const uint16_t x,const uint16_t y, const uint8_t r, const uint8_t g, const  uint8_t b, const uint8_t a)
{
	if (x >= _infoHeader.width || y >= _infoHeader.height)
		throw std::out_of_range("Pixel coordinates are out of bounds");
	if (_infoHeader.bitCount == DEEP_COLOR_BIT_SIZE)
	{
		_pixelData[y * _infoHeader.width + x] = Pixel(r, g, b, a);
	}

	else if (_infoHeader.bitCount == TRUE_COLOR_BIT_SIZE)
	{
		if (a != 0) std::cout << "Pixel " << x << ", " << y << " : The image do not have alpha channel component.\n";
		_pixelData[y * _infoHeader.width + x] = Pixel(r, g, b);
	}
}

/// <summary>
/// Set the pixel at the specified row and column
/// </summary>
/// <param name="x">row</param>
/// <param name="y">column</param>
/// <param name="pixel"></param>
void BMPImage::setPixel(uint16_t x, uint16_t y, const Pixel& pixel)
{
	_pixelData[y * _infoHeader.width + x] = pixel;
}

/// <summary>
/// Resize the image to the specified height and width
/// </summary>
/// <param name="newHeight"></param>
/// <param name="newWidth"></param>
void BMPImage::resize(const int32_t newWidth, const int32_t newHeight)
{
	if (newHeight > 0 && newWidth > 0)
	{
		if (_infoHeader.height != newHeight || _infoHeader.width != newWidth)
		{
			_resizePixelsData(newWidth, newHeight);
			std::cout << "Image resized successfully" << std::endl;
		}
		else
		{
			std::cout << "The image already has the specified dimensions" << std::endl;
		}
	}
	else
	{
		throw std::invalid_argument("Height and width must be greater than 0");
	}
}

/// <summary>
/// Set a new height for the image
/// </summary>
/// <param name="height"></param>
void BMPImage::setHeight(const int32_t height)
{
	resize(height, _infoHeader.width);
}


/// <summary>
/// Set a new width for the image
/// </summary>
/// <param name="width"></param>
void BMPImage::setWidth(int32_t width)
{
	resize(_infoHeader.height, width);
}

/// <summary>
/// Get the resolution of the image
/// </summary>
/// <param name="xPixelsPerMeter"></param>
/// <param name="yPixelsPerMeter"></param>
void BMPImage::getResolution(int32_t& xPixelsPerMeter, int32_t& yPixelsPerMeter) const
{
	xPixelsPerMeter = _infoHeader.xPixelsPerMeter;
	yPixelsPerMeter = _infoHeader.yPixelsPerMeter;
}

/// <summary>
/// Set a new resolution for the image
/// </summary>
/// <param name="xPixelsPerMeter"></param>
/// <param name="yPixelsPerMeter"></param>
void BMPImage::setResolution(const int32_t xPixelsPerMeter,const int32_t yPixelsPerMeter)
{
	_infoHeader.xPixelsPerMeter = xPixelsPerMeter;
	_infoHeader.yPixelsPerMeter = yPixelsPerMeter;
}

/// <summary>
/// Set a new resolution for the image
/// </summary>
/// <param name="resolution"></param>
void BMPImage::setResolution(const int32_t resolution)
{
	setResolution(resolution, resolution);
}

/// <summary>
/// <summary>
/// Multiply the size of the image by a given factor
/// </summary>
/// <param name="factor">Factor to multiply the size by</param>
void BMPImage::multiplySize(float factor)
{

    if (factor == 0)
    {
        throw std::invalid_argument("Factor can not be 0");
    }
	uint8_t reverse = 0;
	if(factor < 0)
	{
		reverse = 1;
		factor = -factor;
		std::cout << "Image reverse...\n";
	}


    const int32_t newWidth = static_cast<int32_t>(_infoHeader.width * factor);
    const int32_t newHeight = static_cast<int32_t>(_infoHeader.height * factor);

    std::vector<Pixel> newPixelData(newHeight * newWidth);

    for (int32_t y = 0; y < newHeight; ++y)
    {
        for (int32_t x = 0; x < newWidth; ++x)
        {
			int32_t oldX;
			int32_t oldY;
			if (reverse == 1)
			{
				oldX = static_cast<int32_t>( _infoHeader.width - (x / factor));
				oldY = static_cast<int32_t>( _infoHeader.height - (y / factor));

			}
			else
			{
				oldX = static_cast<int32_t>(x / factor);
				oldY = static_cast<int32_t>(y / factor);
			}
            if (oldX < _infoHeader.width && oldY < _infoHeader.height)
            {
                newPixelData[y * newWidth + x] = _pixelData[oldY * _infoHeader.width + oldX];
            }
        }
    }

    _infoHeader.width = newWidth;
    _infoHeader.height = newHeight;
    _pixelData = std::move(newPixelData);

    _updateHeaders();
	if (factor > 1)
		std::cout << "Image widen successfully" << std::endl;
	if(factor <=1)
	{
		std::cout << "Image shrinked successfully" << std::endl;
	}
}

std::ostream& operator<<(std::ostream& os, const BMPImage& image)
{
	os << "Image informations : " << std::endl;
	os << " - File size: " << image._fileHeader.fileSize << " bytes" << std::endl;

	os << " - Width: " << image._infoHeader.width << std::endl;
	os << " - Height: " << image._infoHeader.height << std::endl;

	os << " - Bit count: " << image._infoHeader.bitCount << std::endl;

	return os;
}

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
static T ConvertEndian(T value)
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
	uint32_t info_header_size= 0;
	uint32_t compression = 0 ;
	_infoHeader = BMPInfoHeader{
		info_header_size, 0, 0, COLOR_PLANES_NUMBER, bitCount, compression, 0, BM_DEFAULT_RESOLUTION,
		BM_DEFAULT_RESOLUTION, 0, 0
	};
	_v4InfoHeader = BMPV4InfoHeader{
			info_header_size, 0, 0, COLOR_PLANES_NUMBER, bitCount, compression, 0, BM_DEFAULT_RESOLUTION,
			BM_DEFAULT_RESOLUTION, 0, 0, RED_CHANNEL_BIT_MASK, GREEN_CHANNEL_BIT_MASK,
			BLUE_CHANNEL_BIT_MASK, ALPHA_CHANNEL_BIT_MASK, LCS_WINDOWS_COLOR_SPACE, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};

	if (bitCount == TRUE_COLOR_BIT_SIZE)
	{
		_activeHeader = _infoHeader;
		info_header_size = BM_INFO_HEADER_SIZE;
		compression = BI_RGB;
		
	}
	else if (bitCount == DEEP_COLOR_BIT_SIZE)
	{
		_activeHeader = _v4InfoHeader;
		info_header_size = BM_V4_INFO_HEADER_SIZE;
		compression = BI_BITFIELDS;
	}
	else
	{
		throw std::invalid_argument("Image Bit Count not handled");
	}
	_activeHeader.size = info_header_size;
	_activeHeader.compression = compression;
	_fileHeader = BMPFileHeader{
		BM_SIGNATURE, BM_FILE_HEADER_SIZE + info_header_size, 0, 0, BM_FILE_HEADER_SIZE + info_header_size
	};
}

/// <summary>
/// Constructor with width and height
/// </summary>
/// <param name="width"></param>
/// <param name="height"></param>
/// <param name="bitCount">Color Depth</param>
BMPImage::BMPImage(int32_t width, int32_t height, uint16_t bitCount) : BMPImage(bitCount)
{
	_activeHeader.width = width;
	_activeHeader.height = height;
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
/// load an image from another image object
/// </summary>
/// <param name="other"></param>
BMPImage::BMPImage(const BMPImage& other)
{
	_fileHeader = other._fileHeader;
	_infoHeader = other._infoHeader;
	_v4InfoHeader = other._v4InfoHeader;
	_activeHeader = other._activeHeader;
	_pixelData = other._pixelData;
}

/// <summary>
/// Destroy the image object
/// </summary>
BMPImage::~BMPImage() = default;


BMPImage& BMPImage::operator=(const BMPImage& other) {
	if (this == &other) {
		return *this; 
	}
	_fileHeader = other._fileHeader;
	_infoHeader = other._infoHeader;
	_v4InfoHeader = other._v4InfoHeader;
	_activeHeader = other._activeHeader;
	_pixelData = other._pixelData;

	return *this;
}

bool BMPImage::_isTrueColor() const
{
	return _activeHeader.bitCount == TRUE_COLOR_BIT_SIZE;
}

bool BMPImage::_isDeepColor() const
{
	return _activeHeader.bitCount == DEEP_COLOR_BIT_SIZE;
}

uint16_t BMPImage::_getByteCount() const
{
	
	if (_activeHeader.bitCount == DEEP_COLOR_BIT_SIZE)
		return 4;
	if (_activeHeader.bitCount == TRUE_COLOR_BIT_SIZE)
		return 3;
	if (_activeHeader.bitCount == GRAY_SCALE_BIT_SIZE)
		return 4;
	if (_activeHeader.bitCount == MONOCHROME_BIT_SIZE)
		return 1;
	throw std::runtime_error("Invalid bit count");
}

void BMPImage::_readHeaders(std::ifstream& file)
{
	// Read the file header
	file.read(reinterpret_cast<char*>(&_fileHeader), sizeof(_fileHeader));
	// Check if it's a BMP file by looking for the "BM" signature
	if (_fileHeader.fileType != BM_SIGNATURE)
	{
		throw std::runtime_error("File is not a BMP file.");
	}
	// Read the info header
	if (_fileHeader.offsetData == BM_INFO_HEADER_SIZE + BM_FILE_HEADER_SIZE)
	{
		file.read(reinterpret_cast<char*>(&_infoHeader), sizeof(_infoHeader));
		// Check if the image is uncompressed
		if (_infoHeader.compression != BI_RGB)
		{
			throw std::runtime_error("Image is compressed. This is not handled by the program");
		}
		// Check if the image bit count is valid
		if (_infoHeader.bitCount != TRUE_COLOR_BIT_SIZE)
		{
			throw std::runtime_error("Image bit count is not valid.");
		}
		_activeHeader = _infoHeader;

	}
	// Read the V4 info header
	else if (_fileHeader.offsetData == BM_V4_INFO_HEADER_SIZE + BM_FILE_HEADER_SIZE)
	{
		file.read(reinterpret_cast<char*>(&_v4InfoHeader), sizeof(_v4InfoHeader));
		// Check if the image is uncompressed
		if (_v4InfoHeader.compression != BI_BITFIELDS)
		{
			throw std::runtime_error("Image is compressed. This is not handled by the program");
		}
		// Check if the image bit count is valid
		if (_v4InfoHeader.bitCount != DEEP_COLOR_BIT_SIZE)
		{
			throw std::runtime_error("Image bit count is not valid.");
		}
		_activeHeader = _v4InfoHeader;

	}
}

void BMPImage::_readPixels(std::ifstream& file)
{
	// Pixel data
	for (int i = 0; i < _activeHeader.height; i++)
	{
		for (int j = 0; j < _activeHeader.width; j++)
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
	file.write(reinterpret_cast<const char*>(&_fileHeader), sizeof(_fileHeader));

	if (_isTrueColor())
	{
		file.write(reinterpret_cast<const char*>(&_infoHeader), sizeof(_infoHeader));
	}
	else if (_isDeepColor())
	{
		file.write(reinterpret_cast<const char*>(&_v4InfoHeader), sizeof(_v4InfoHeader));
	}
	else
	{
		throw std::runtime_error("Invalid bit count");
	}
}


void BMPImage::_writePixels(std::ofstream& file) const
{
	// Write the pixel data
	for (int i = 0; i < _activeHeader.height; i++)
	{
		for (int j = 0; j < _activeHeader.width; j++)
		{
			Pixel pixel = _pixelData[i * _activeHeader.width + j];
			std::streamsize pixelSize = _getByteCount();
			auto pixel_ptr = std::make_unique<uint8_t[]>(pixelSize);
			pixel_ptr[0] = pixel.getRed();
			pixel_ptr[1] = pixel.getGreen();
			pixel_ptr[2] = pixel.getBlue();
			if (_isDeepColor())
			{
				pixel_ptr[3] = pixel.getAlpha();
			}
			std::swap(pixel_ptr[0], pixel_ptr[2]); // Swap red and blue channels
			file.write(reinterpret_cast<const char*>(pixel_ptr.get()), pixelSize);
		}
		
		// Add padding if needed
		int paddingSize = (4 - (_activeHeader.width * _getByteCount()) % 4) % 4;
		uint8_t paddingData[4] = {0, 0, 0, 0};
		file.write(reinterpret_cast<const char*>(paddingData), paddingSize);
	}
}

void BMPImage::_updateHeaders()
{
	_activeHeader.sizeImage = _activeHeader.width * _activeHeader.height * _getByteCount();
	_fileHeader.fileSize = BM_FILE_HEADER_SIZE + BM_INFO_HEADER_SIZE + _activeHeader.sizeImage;
	_fileHeader.offsetData = BM_FILE_HEADER_SIZE + BM_INFO_HEADER_SIZE;

}

void BMPImage::_resizePixelsData(int32_t newWidth, int32_t newHeight)
{
	std::vector<Pixel> newPixelData(newHeight * newWidth);

	{
		const int32_t minHeight = std::min(_activeHeader.height, newHeight);
		const int32_t minWidth = std::min(_activeHeader.width, newWidth);
		const int32_t maxHeight = std::max(_activeHeader.height, newHeight);
		const int32_t maxWidth = std::max(_activeHeader.width, newWidth);
		for (int i = 0; i < maxHeight; i++)
		{
			for (int j = 0; j < maxWidth; j++)
			{
				if (i < minHeight && j < minWidth)
				{
					newPixelData[i * newWidth + j] = _pixelData[i * _activeHeader.width + j];
				}
			}
		}
		// update the headers
		_activeHeader.height = newHeight;
		_activeHeader.width = newWidth;
		// resize the pixel data
		_pixelData.resize(newHeight * newWidth);
		// copy the data of the new pixel data to the pixel data
		Pixel* newPixelDataPtr = newPixelData.data();
		std::copy(newPixelDataPtr, newPixelDataPtr + newHeight * newWidth, _pixelData.data());
	}

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
	return _activeHeader.width;
}

uint32_t BMPImage::getHeight() const
{
	return _activeHeader.height;
}

/// <summary>
/// Get the pixel at the specified row and column
/// </summary>
/// <param name="x">row</param>
/// <param name="y">column</param>
/// <returns></returns>
Pixel BMPImage::getPixel(const uint16_t x, const uint16_t y) const
{
	if (x >= _activeHeader.width || y >= _activeHeader.height)
	{
		throw std::out_of_range("Pixel coordinates are out of bounds");
	}
	Pixel pixel = _pixelData[y * _activeHeader.width + x];
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
	if (x >= _activeHeader.width || y >= _activeHeader.height)
		throw std::out_of_range("Pixel coordinates are out of bounds");
	if (_activeHeader.bitCount == DEEP_COLOR_BIT_SIZE)
	{
		_pixelData[y * _activeHeader.width + x] = Pixel(r, g, b, a);
	}

	else if (_activeHeader.bitCount == TRUE_COLOR_BIT_SIZE)
	{
		if (a != 0) std::cout << "Pixel " << x << ", " << y << " : The image do not have alpha channel component.\n";
		_pixelData[y * _activeHeader.width + x] = Pixel(r, g, b);
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
	_pixelData[y * _activeHeader.width + x] = pixel;
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
		if (_activeHeader.height != newHeight || _activeHeader.width != newWidth)
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
	resize(height, _activeHeader.width);
}


/// <summary>
/// Set a new width for the image
/// </summary>
/// <param name="width"></param>
void BMPImage::setWidth(int32_t width)
{
	resize(_activeHeader.height, width);
}

/// <summary>
/// Get the resolution of the image
/// </summary>
/// <param name="xPixelsPerMeter"></param>
/// <param name="yPixelsPerMeter"></param>
void BMPImage::getResolution(int32_t& xPixelsPerMeter, int32_t& yPixelsPerMeter) const
{
	xPixelsPerMeter = _activeHeader.xPixelsPerMeter;
	yPixelsPerMeter = _activeHeader.yPixelsPerMeter;
}

/// <summary>
/// Set a new resolution for the image
/// </summary>
/// <param name="xPixelsPerMeter"></param>
/// <param name="yPixelsPerMeter"></param>
void BMPImage::setResolution(const int32_t xPixelsPerMeter,const int32_t yPixelsPerMeter)
{
	_activeHeader.xPixelsPerMeter = xPixelsPerMeter;
	_activeHeader.yPixelsPerMeter = yPixelsPerMeter;
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


    const int32_t newWidth = static_cast<int32_t>(_activeHeader.width * factor);
    const int32_t newHeight = static_cast<int32_t>(_activeHeader.height * factor);

    std::vector<Pixel> newPixelData(newHeight * newWidth);

    for (int32_t y = 0; y < newHeight; ++y)
    {
        for (int32_t x = 0; x < newWidth; ++x)
        {
			int32_t oldX;
			int32_t oldY;
			if (reverse == 1)
			{
				oldX = static_cast<int32_t>( _activeHeader.width - (x / factor));
				oldY = static_cast<int32_t>( _activeHeader.height - (y / factor));

			}
			else
			{
				oldX = static_cast<int32_t>(x / factor);
				oldY = static_cast<int32_t>(y / factor);
			}
            if (oldX < _activeHeader.width && oldY < _activeHeader.height)
            {
                newPixelData[y * newWidth + x] = _pixelData[oldY * _activeHeader.width + oldX];
            }
        }
    }

    _activeHeader.width = newWidth;
    _activeHeader.height = newHeight;
    _pixelData = std::move(newPixelData);

    _updateHeaders();
	if (factor > 1)
		std::cout << "Image widen successfully" << std::endl;
	if(factor <=1)
	{
		std::cout << "Image shrinked successfully" << std::endl;
	}
}

/// <summary>
/// Generate mandelbrot fractal
/// </summary>
BMPImage BMPImage::Fractal::mandelbrot(const int32_t width, const int32_t height, const int16_t iterations)
{
    // Mandelbrot fractal
    const float aspectRatio = static_cast<float>(width) / height;
    const float scale = 3.5f / std::min(width, height); // scale factor to fit the fractal within the image dimensions
    // offset to center the fractal on the image
    const float offsetX = -2.5f * aspectRatio;
    constexpr float offsetY = -1.75f;
	BMPImage image(width, height, TRUE_COLOR_BIT_SIZE);
	for (int32_t y = 0; y < height; ++y)
	{
		for (int32_t x = 0; x < width; ++x)
		{
			float zx = 0;
			float zy = 0;
			const float cx = (x * scale / aspectRatio) + offsetX;
			const float cy = y * scale + offsetY;
			int16_t i = 0;
			for (; i < iterations; ++i)
			{
				const float temp = zx * zx - zy * zy + cx;
				zy = 2 * zx * zy + cy;
				zx = temp;
				if (zx * zx + zy * zy > 4)
				{
					break;
				}
			}
			const uint8_t r = static_cast<uint8_t>(255 * i / iterations);
			const uint8_t g = static_cast<uint8_t>(255 * i / iterations);
			const uint8_t b = static_cast<uint8_t>(255 * i / iterations);
			image.setPixel(x, y, r, g, b);
		}
	}
	std::cout << "Mandelbrot fractal generated successfully" << std::endl;
	return image;
}


std::ostream& operator<<(std::ostream& os, const BMPImage& image)
{
	os << "Image informations : " << std::endl;
	os << " - File size: " << image._fileHeader.fileSize << " bytes" << std::endl;

	os << " - Width: " << image._activeHeader.width << std::endl;
	os << " - Height: " << image._activeHeader.height << std::endl;

	os << " - Bit count: " << image._activeHeader.bitCount << std::endl;

	return os;
}

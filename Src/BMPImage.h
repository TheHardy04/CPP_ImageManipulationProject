#pragma once
#include <vector>
#include "Pixel.h"



// All of this information is based on the BMP file format : https://en.wikipedia.org/wiki/BMP_file_format
// And for the 32bpp format  : https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapv4heade
class BMPImage
{
	
#pragma pack(push, 1)  // avoid padding
	// BMP file header(14 bytes)
	struct BMPFileHeader {
		uint16_t fileType;		 // File type always BM
		uint32_t fileSize;       // Size of the file (in bytes)
		uint16_t reserved1;      // Reserved, must be 0
		uint16_t reserved2;      // Reserved, must be 0
		uint32_t offsetData;     // Start position of pixel data (bytes from the beginning of the file)
	}_fileHeader;

	// BMP info header(40 bytes)
	struct BMPInfoHeader {
		uint32_t size;				// Size of this header (in bytes)
		int32_t width;				// width of bitmap in pixels
		int32_t height;				// height of bitmap in pixels
		uint16_t planes;			// No. of color planes being used
		uint16_t bitCount;			// No. of bits per pixel (color depth). Usually 1, 4, 8, 16, 24, 32
		uint32_t compression;		// Compression method
		uint32_t sizeImage;			// Size of raw bitmap data
		int32_t xPixelsPerMeter;	// Horizontal resolution
		int32_t yPixelsPerMeter;	// Vertical resolution
		uint32_t colorsUsed;		// No. of colors in the color palette
		uint32_t colorsImportant;	// No. of important colors
	}_infoHeader;

	// BMP V4 info header(108 bytes)
	// See https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapv4header

    struct BMPV4InfoHeader : BMPInfoHeader {
        uint32_t redMask;             // Mask identifying bits of red component
        uint32_t greenMask;           // Mask identifying bits of green component
        uint32_t blueMask;            // Mask identifying bits of blue component
        uint32_t alphaMask;           // Mask identifying bits of alpha component
        std::string csType;           // Color space type
        int32_t redX;                 // X coordinate of red endpoint
        int32_t redY;                 // Y coordinate of red endpoint
        int32_t redZ;                 // Z coordinate of red endpoint
        int32_t greenX;               // X coordinate of green endpoint
        int32_t greenY;               // Y coordinate of green endpoint
        int32_t greenZ;               // Z coordinate of green endpoint
        int32_t blueX;                // X coordinate of blue endpoint
        int32_t blueY;                // Y coordinate of blue endpoint
        int32_t blueZ;                // Z coordinate of blue endpoint
        uint32_t gammaRed;            // Gamma red coordinate scale value
        uint32_t gammaGreen;          // Gamma green coordinate scale value
        uint32_t gammaBlue;           // Gamma blue coordinate scale value
    }_v4InfoHeader;
#pragma pack(pop)
	BMPInfoHeader& _activeHeader = _infoHeader;

	std::vector<Pixel> _pixelData;	// 2D vector to store pixels

	bool _isTrueColor() const;
	bool _isDeepColor() const;
	uint16_t _getByteCount() const;				
	void _readHeaders(std::ifstream& file);
	void _readPixels(std::ifstream& file);
	void _writeHeaders(std::ofstream& file) const;
	void _writePixels(std::ofstream& file) const;
	void _updateHeaders();
	void _resizePixelsData(int32_t newWidth, int32_t newHeight);

	
public:

	// Constants
	static constexpr uint16_t BM_SIGNATURE = 0x4D42;

	static constexpr uint32_t BM_FILE_HEADER_SIZE = 14;
	static constexpr uint32_t BM_INFO_HEADER_SIZE = 40;
	static constexpr uint32_t BM_V4_INFO_HEADER_SIZE = 108;
	static constexpr uint32_t BM_V5_INFO_HEADER_SIZE = 124;

	static constexpr uint16_t COLOR_PLANES_NUMBER = 1;
	static constexpr uint32_t BI_RGB = 0;
	static constexpr uint32_t BI_BITFIELDS = 3;

	static constexpr int32_t BM_DEFAULT_RESOLUTION = 1;

	static constexpr uint32_t RED_CHANNEL_BIT_MASK = 0x00FF0000;
	static constexpr uint32_t GREEN_CHANNEL_BIT_MASK = 0x0000FF00;
	static constexpr uint32_t BLUE_CHANNEL_BIT_MASK = 0x000000FF;
	static constexpr uint32_t ALPHA_CHANNEL_BIT_MASK = 0xFF000000;
    static constexpr const char* LCS_WINDOWS_COLOR_SPACE = "Win ";

	static constexpr uint16_t DEEP_COLOR_BIT_SIZE = 32;
	static constexpr uint16_t TRUE_COLOR_BIT_SIZE = 24;
	static constexpr uint16_t GRAY_SCALE_BIT_SIZE = 8;
	static constexpr uint16_t MONOCHROME_BIT_SIZE = 1;

	BMPImage(uint16_t bitCount = TRUE_COLOR_BIT_SIZE);
	BMPImage(int32_t width, int32_t height, uint16_t bitCount = TRUE_COLOR_BIT_SIZE);
	BMPImage(const char* filename);
	BMPImage(const BMPImage& other);
	~BMPImage();
	BMPImage& operator=(const BMPImage& other);

	static void openImage(const std::string& filename);

	void load(const char* filename);
	void save(const char* filename) const;
	uint32_t getWidth() const;
	uint32_t getHeight() const;
	Pixel getPixel(uint16_t x, uint16_t y) const;
	void setPixel(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0);
	void setPixel(uint16_t x, uint16_t y, const Pixel& pixel);
	void resize(int32_t newWidth, int32_t newHeight);
	void setWidth(int32_t width);
	void setHeight(int32_t height);
	void getResolution(int32_t& xPixelsPerMeter, int32_t& yPixelsPerMeter) const;
	void setResolution(int32_t xPixelsPerMeter, int32_t yPixelsPerMeter);
	void setResolution(int32_t resolution);
	void multiplySize(float factor);
	class Fractal
	{
	public:
		static BMPImage mandelbrot(int32_t width, int32_t height, int16_t iterations);
	};
 


	friend std::ostream& operator<<(std::ostream& os, const BMPImage& image);

};
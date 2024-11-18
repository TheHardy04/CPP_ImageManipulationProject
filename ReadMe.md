# BMP Image Manipulation Project

## Description

This project is a student project aiming to introduce to C++ data structures. It allows manipulation of BMP images, including loading, modifying pixels, and saving images.

## Features

- Load a BMP image from a file.
- Save a BMP image to a file.
- Access and modify the pixels of an image.
- Resize an image.
- Change the resolution of an image.

## Usage

### Load an Image

```cpp
BMPImage image("Images/bmp_24.bmp"); stdcout << image << stdendl;
```

### Save an Image

```cpp
image.save("Images/bmp_24_copy.bmp");
```

### Access and Modify Pixels

```cpp
image(0, 0) = Pixel(255, 0, 0);
```

### Resize an Image

```cpp
image.resize(100, 100);
```

### Change the Resolution of an Image

```cpp
image.changeResolution(100, 100);
```

## Demo
The main.cpp file contains a demo of the project.


## Compilation

This project uses C++14. To compile the project, use the following command:

```bash
g++ -std=c++14 -o BMPImage main.cpp BMPImage.cpp Pixel.cpp
```

## License
[MIT](https://choosealicense.com/licenses/mit/)

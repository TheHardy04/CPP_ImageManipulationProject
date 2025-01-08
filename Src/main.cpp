#include "BMPImage.h"
#include "Pixel.h"
#include <iostream>
#include <vector>

// Cross-platform terminal handling
#ifdef _WIN32
#include <conio.h>  // For _getch() on Windows
#define CLEAR_SCREEN "cls"
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#include <filesystem>
#include <cstdio>
namespace fs = std::filesystem;
#define CLEAR_SCREEN "clear"
#endif

namespace 
{
	void displayLogo() {
		std::cout << R"(
      ___           ___           ___                                ___           ___           ___           ___              
     /\  \         /\  \         /\  \                   ___        /\__\         /\  \         /\  \         /\  \             
    /::\  \       /::\  \       /::\  \                 /\  \      /::|  |       /::\  \       /::\  \       /::\  \            
   /:/\:\  \     /:/\:\  \     /:/\:\  \                \:\  \    /:|:|  |      /:/\:\  \     /:/\:\  \     /:/\:\  \           
  /:/  \:\  \   /::\~\:\  \   /::\~\:\  \               /::\__\  /:/|:|__|__   /::\~\:\  \   /:/  \:\  \   /::\~\:\  \          
 /:/__/ \:\__\ /:/\:\ \:\__\ /:/\:\ \:\__\           __/:/\/__/ /:/ |::::\__\ /:/\:\ \:\__\ /:/__/_\:\__\ /:/\:\ \:\__\         
 \:\  \  \/__/ \/__\:\/:/  / \/__\:\/:/  /          /\/:/  /    \/__/~~/:/  / \/__\:\/:/  / \:\  /\ \/__/ \:\~\:\ \/__/         
  \:\  \            \::/  /       \::/  /           \::/__/           /:/  /       \::/  /   \:\ \:\__\    \:\ \:\__\           
   \:\  \            \/__/         \/__/             \:\__\          /:/  /        /:/  /     \:\/:/  /     \:\ \/__/           
    \:\__\                                            \/__/         /:/  /        /:/  /       \::/  /       \:\__\             
     \/__/                                                          \/__/         \/__/         \/__/         \/__/             
    )" << "\n";
	}

    void waitForKey()
	{
		std::cout << "Press enter to continue...";
		std::cin.get();
	}

#if not defined(_WIN32)
    // Linux-specific terminal functions
    void disableCanonicalMode(struct termios& oldSettings) {
        struct termios newSettings;
        tcgetattr(STDIN_FILENO, &oldSettings);  // Get current terminal settings
        newSettings = oldSettings;
        newSettings.c_lflag &= ~(ICANON | ECHO);  // Disable canonical mode and echo
        tcsetattr(STDIN_FILENO, TCSANOW, &newSettings);
    }

    void restoreTerminalSettings(const struct termios& oldSettings) {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldSettings);
    }

    int getKeyPress() {
        int ch = getchar();
        if (ch == 27) {  // Check for escape sequence (arrow keys)
            getchar();   // Skip '['
            ch = getchar();
        }
        return ch;
    }
#endif

    // Cross-platform menu function
    int selectOption(const std::vector<std::string>& options) {
        int current = 0;

#ifdef _WIN32
        while (true) {
            system(CLEAR_SCREEN);
			std::cout << options[0] << std::endl;
			std::cout << "Select an option:" << std::endl;
            for (size_t i = 1; i < options.size(); ++i) {
                if (i == current) {
                    std::cout << "> " << options[i] << " <" << std::endl;  // Highlight selection
                }
                else {
                    std::cout << "  " << options[i] << std::endl;
                }
            }

            int key = _getch();
            if (key == 224) {  // Arrow key prefix
                key = _getch();
                if (key == 72) {  // Up arrow
                    current = (current - 1 + options.size()) % options.size();
                }
                else if (key == 80) {  // Down arrow
                    current = (current + 1) % options.size();
                }
            }
            else if (key == 13) {  // Enter key
                break;
            }
        }
#else
        struct termios oldSettings;
        disableCanonicalMode(oldSettings);

        while (true) {
            system(CLEAR_SCREEN);
			std::cout << options[0] << std::endl;
			std::cout << "Select an option:" << std::endl;
            for (size_t i = 1; i < options.size(); ++i) {
                if (i == current) {
                    std::cout << "> " << options[i] << " <" << std::endl;
                }
                else {
                    std::cout << "  " << options[i] << std::endl;
                }
            }

            int key = getKeyPress();
            if (key == 'A') {  // Up arrow
                current = (current - 1 + options.size()) % options.size();
            }
            else if (key == 'B') {  // Down arrow
                current = (current + 1) % options.size();
            }
            else if (key == '\n') {  // Enter key
                break;
            }
        }

        restoreTerminalSettings(oldSettings);
#endif

        return current;
    }

    std::vector<std::string> getFilesInDirectory(const std::string& directoryPath, std::string extension = "") {
        std::vector<std::string> fileNames;
		if (extension[0] == '.' && extension.size() > 1)
		{
			extension = extension.substr(1);
		}

    #ifdef _WIN32
        // Windows-specific implementation
        WIN32_FIND_DATA findFileData;
        HANDLE hFind = FindFirstFile((directoryPath + "\\*" + extension).c_str(), &findFileData);

        if (hFind == INVALID_HANDLE_VALUE) {
            std::cerr << "Error opening directory: " << directoryPath << std::endl;
            return fileNames;
        }

        do {
            const std::string fileName = findFileData.cFileName;
            if (fileName != "." && fileName != "..") {
                fileNames.push_back(fileName);
            }
        } while (FindNextFile(hFind, &findFileData) != 0);

        FindClose(hFind);

    #else
        // Linux/macOS implementation using C++17 filesystem
        try {
            for (const auto& entry : fs::directory_iterator(directoryPath)) {
                if (entry.is_regular_file() && entry.path().extension() == "." + extension) {  // Only add regular files with the specified extension
                    fileNames.push_back(entry.path().filename().string());
                }
            }
        }
        catch (const fs::filesystem_error& e) {
            std::cerr << "Error accessing directory: " << e.what() << std::endl;
        }
    #endif

        return fileNames;
    }


    BMPImage openImage()
	{
		// File selection menu
        std::string filename;
        std::string path = "Images";

        std::vector<std::string> fileOptions = {"File to open"};
		std::vector<std::string> files = getFilesInDirectory(path, ".bmp");
		fileOptions.insert(fileOptions.end(), files.begin(), files.end());
        fileOptions.push_back("Demo Images");
		int choice = selectOption(fileOptions);
		if (choice == fileOptions.size() - 1)
		{
            path += "/DemoImages";
			std::vector<std::string> demoImages = {
				"File to open"
			};
			std::vector<std::string> demoFiles = getFilesInDirectory(path,".bmp");
			demoImages.insert(demoImages.end(), demoFiles.begin(), demoFiles.end());
			int demoChoice = selectOption(demoImages);
			filename = path + "/" +demoImages[demoChoice];
		}
		else
		{
			filename = path + "/" + fileOptions[choice];
		}
		std::cout << "Opening " << filename << "...\n";
        BMPImage image(filename.c_str());
        return image;
	}

    void exitProgram()
	{
		std::cout << "Exiting program...\n";
		exit(0);
	}

	void save(const BMPImage& image)
	{
        std::string filename;
        std::cout << "Enter the filename: ";
        std::cin >> filename;
        filename = "Images/" + filename;
		image.save(filename.c_str());
		std::cout << "Image saved successfully\n";
	}


	void manipulate_image(BMPImage& image)
	{
		std::vector<std::string> options = {
            "What to do ?",
            "Open Image",
			"Apply a factor to the size",
			"Manual Resize",
			"Save",
			"Return to menu",
		};
        bool saved = false;
        while (true)
        {
			int choice = selectOption(options);
            system(CLEAR_SCREEN);
            if (choice == 1)
            {
				float factor;
				std::cout << "Enter the factor: ";
				std::cin >> factor;
				image.multiplySize(factor);
			}
			else if (choice == 2)
			{
				int width, height;
				std::cout << "Enter the new width: ";
				std::cin >> width;
				std::cout << "Enter the new height: ";
				std::cin >> height;
				image.resize(width, height);
			}
			else if (choice == 3)
			{
				save(image);
				saved = true;
            }
			if (choice == 4)
			{
                if (!saved)
                {
                    std::vector<std::string> saveOptions = {
						"Do you want to save the image before exiting?",
                        "Yes",
                        "No",
                    };
                    int saveChoice = selectOption(saveOptions);
                    if (saveChoice == 1)
                    {
                        save(image);
                    }
                }
				break;
			}
        }
	}

	BMPImage generateImage()
	{
		int width, height;
		std::cout << "Enter the width: \n";
		std::cin >> width;
		std::cout << "Enter the height: \n";
		std::cin >> height;
#ifndef _WIN32
        waitForKey();
#endif
        system(CLEAR_SCREEN);
        std::vector<std::string> options = {
            "Choose an option",
            "Do nothing",
            "Fractal"
        };
		int choice = selectOption(options);
		BMPImage image(width, height);
		if (choice == 2)
		{
            image = BMPImage::Fractal::mandelbrot(width,height,150);
		}
		return image;
	}
}


int main() {
	displayLogo();
    waitForKey();

    
    std::vector<std::string> options = {
        "Welcome to this BMP Image Manipulation Program",
		"Open Existing Image",
		"Generate New Image",
		"Exit",
	};
    BMPImage currentImage;
	while (true) {
		int choice = selectOption(options);
		system(CLEAR_SCREEN);
        if (choice == 1) {
        	currentImage = openImage();
            manipulate_image(currentImage);
        }
        else if (choice == 2) {
			currentImage = generateImage();
			manipulate_image(currentImage);
        }
        else if (choice == 3) {
            exitProgram();
        }
	}
}




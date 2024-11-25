#include "BMPImage.h"
#include "Pixel.h"
#include <iostream>

// Cross-platform terminal handling
#ifdef _WIN32
#include <conio.h>  // For _getch() on Windows
#define CLEAR_SCREEN "cls"
#else
#include <termios.h>
#include <unistd.h>
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
    
    void waitForKey()
	{
		std::cout << "Press enter to continue...";
		std::cin.get();
	}

    BMPImage openImage()
	{
        std::string filename;
        std::cout << "Enter the filename: ";
        std::cin >> filename;
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
		image.save(filename.c_str());
		std::cout << "Image saved successfully\n";
	}

	void manipulate_image(BMPImage& image)
	{
		std::vector<std::string> options = {
            "What to do ?",
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
                    if (saveChoice == 0)
                    {
                        save(image);
                    }
                }
				break;
			}
        }
	}
}


int main() {
	/*std::cout << "Image from File :" << std::endl;
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
	std::cout << std::endl;*/

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
            
        }
        else if (choice == 3) {
            exitProgram();
        }
	}
}




/*
 * Created by Brett Terpstra 6920201 on 14/10/22.
 * Copyright (c) Brett Terpstra 2022 All Rights Reserved
 */
#include "image/image.h"
#include <ios>
#include <fstream>
#include <image/stb_image_write.h>

namespace Raytracing {

    Image::Image(int width, int height) : width(width), height(height) {
        pixelData = new vec4[width * height];
    }

    Image::Image(const Image& image) : width(image.width), height(image.height) {
        pixelData = new vec4[image.width * image.height];
        for (int i = 0; i < image.width; i++) {
            for (int j = 0; j < image.height; j++) {
                this->setPixelColor(i, j, image.pixelData[i * image.width + j]);
            }
        }
    }

    int Image::getPixelR(int x, int y) const {
        // values are stored as a floating point number [0, 1)
        // but most formats want an int [0, 255]
        return (int) (255.999 * getPixelColor(x, y).r());
    }

    int Image::getPixelG(int x, int y) const {
        return (int) (255.999 * getPixelColor(x, y).g());
    }

    int Image::getPixelB(int x, int y) const {
        return (int) (255.999 * getPixelColor(x, y).b());
    }

    int Image::getPixelA(int x, int y) const {
        return (int) (255.999 * getPixelColor(x, y).a());
    }

    Image::~Image() {
        delete[](pixelData);
    }

    void ImageOutput::write(const std::string& file, const std::string& formatExtension) {
        auto lowerExtension = Raytracing::String::toLowerCase(formatExtension);
        auto fullFile = file + lowerExtension;

        if (!lowerExtension.ends_with("hdr")) {
            // unfortunately we do have to put the data into a format that STB can read
            int data[image.getWidth() * image.getHeight() * 3];
            int pixelIndex = 0;
            for (int i = 0; i < image.getWidth(); i++) {
                for (int j = 0; j < image.getHeight(); j++) {
                    data[pixelIndex++] = image.getPixelR(i, j);
                    data[pixelIndex++] = image.getPixelR(i, j);
                    data[pixelIndex++] = image.getPixelR(i, j);
                }
            }

            // Writing a PPM was giving me issues, so I switched to using STB Image Write
            // It's a single threaded, public domain header only image writing library
            // I didn't want to use an external lib for this, however since it is public domain
            // I've simply included it in the include directory.
            if (lowerExtension.ends_with("bmp")) {
                stbi_write_bmp(fullFile.c_str(), image.getWidth(), image.getHeight(), 3, data);
            } else if (lowerExtension.ends_with("png")) {
                stbi_write_png(fullFile.c_str(), image.getWidth(), image.getHeight(), 3, data, 3 * sizeof(int));
            } else if (lowerExtension.ends_with("jpg")) {
                stbi_write_jpg(fullFile.c_str(), image.getWidth(), image.getHeight(), 3, data, 90);
            } else
                throw std::runtime_error("Invalid format! Please use ppm, png, or jpg");
        } else {
            // the TODO: here is to check if HDR is in [0,1] or if we need to transform the value.
            float data[image.getWidth() * image.getHeight() * 3];
            int pixelIndex = 0;
            for (int i = 0; i < image.getWidth(); i++) {
                for (int j = 0; j < image.getHeight(); j++) {
                    data[pixelIndex++] = (float) image.getPixelColor(i, j).r();
                    data[pixelIndex++] = (float) image.getPixelColor(i, j).g();
                    data[pixelIndex++] = (float) image.getPixelColor(i, j).b();
                }
            }
            stbi_write_hdr(fullFile.c_str(), image.getWidth(), image.getHeight(), 3, data);
        }
    }
}
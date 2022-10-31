/*
 * Created by Brett Terpstra 6920201 on 14/10/22.
 * Copyright (c) Brett Terpstra 2022 All Rights Reserved
 */
#include "engine/image/image.h"
#include <ios>
#include <fstream>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "engine/image/stb_image_write.h"
#include "engine/image/stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "engine/image/stb_image_resize.h"

namespace Raytracing {

    Image::Image(unsigned long width, unsigned long height) : width(width), height(height), _width(width-1), _height(height-1) {
        pixelData = new Vec4[(width + 1) * (height + 1)];
    }

    Image::Image(const Image& image) : width(image.width), height(image.height), _width(image._width), _height(image._height) {
        pixelData = new Vec4[(image.width + 1) * (image.height + 1)];
        for (int i = 0; i < image.width; i++) {
            for (int j = 0; j < image.height; j++) {
                this->setPixelColor(i, j, image.pixelData[i * image.height + j]);
            }
        }
    }

    int Image::getPixelR(int x, int y) const {
        // values are stored as a floating point number [0, 1)
        // but most formats want an int [0, 255]
        return int(255.0 * getPixelColor(x, y).r());
    }

    int Image::getPixelG(int x, int y) const {
        return int(255.0 * getPixelColor(x, y).g());
    }

    int Image::getPixelB(int x, int y) const {
        return int(255.0 * getPixelColor(x, y).b());
    }

    int Image::getPixelA(int x, int y) const {
        return int(255.0 * getPixelColor(x, y).a());
    }

    Image::~Image() {
        delete[](pixelData);
    }

    void ImageOutput::write(const std::string& file, const std::string& formatExtension) {
        auto lowerExtension = Raytracing::String::toLowerCase(formatExtension);
        auto fullFile = file + "." + lowerExtension;

        if (!lowerExtension.ends_with("hdr")) {
            // unfortunately we do have to put the data into a format that STB can read
            auto* data = new unsigned char[(unsigned long)(image.getWidth()) * (unsigned long)image.getHeight() * 3];
            int pixelIndex = 0;
            for (int j = image.getHeight()-1; j >= 0; j--) {
                for (int i = 0; i < image.getWidth(); i++) {
                    data[pixelIndex++] = image.getPixelR(i, j);
                    data[pixelIndex++] = image.getPixelG(i, j);
                    data[pixelIndex++] = image.getPixelB(i, j);
                }
            }

            // Writing a PPM was giving me issues, so I switched to using STB Image Write
            // It's a single threaded, public domain header only image writing library
            // I didn't want to use an external lib for this, however since it is public domain
            // I've simply included it in the include directory.
            if (lowerExtension.ends_with("bmp")) {
                stbi_write_bmp(fullFile.c_str(), image.getWidth(), image.getHeight(), 3, data);
            } else if (lowerExtension.ends_with("png")) {
                // stride here isn't clearly defined in the docs for some reason,
                // but it's just the image's width times the number of channels
                stbi_write_png(fullFile.c_str(), image.getWidth(), image.getHeight(), 3, data, image.getWidth() * 3);
            } else if (lowerExtension.ends_with("jpg") || lowerExtension.ends_with("jpeg")) {
                stbi_write_jpg(fullFile.c_str(), image.getWidth(), image.getHeight(), 3, data, 90);
            } else
                throw std::runtime_error("Invalid format! Please use bmp, png, or jpg");
            delete[](data);
        } else {
            // the TODO: here is to check if HDR is in [0,1] or if we need to transform the value.
            auto* data = new float[image.getWidth() * image.getHeight() * 3];
            int pixelIndex = 0;
            for (int i = 0; i < image.getWidth(); i++) {
                for (int j = 0; j < image.getHeight(); j++) {
                    data[pixelIndex++] = (float) image.getPixelColor(i, j).r();
                    data[pixelIndex++] = (float) image.getPixelColor(i, j).g();
                    data[pixelIndex++] = (float) image.getPixelColor(i, j).b();
                }
            }
            stbi_write_hdr(fullFile.c_str(), image.getWidth(), image.getHeight(), 3, data);
            delete[](data);
        }
    }
    ImageInput::ImageInput(const std::string& image) {
        data = stbi_load(image.c_str(), &width, &height, &channels, 4);
    }
    unsigned long* ImageInput::getImageAsIconBuffer() {
        const int size = 32;
        unsigned char newData[size * size * channels];
        auto* returnData = new unsigned long[size * size + 2];
        stbir_resize_uint8(data, width, height, 0, newData, size, size, 0, channels);
        int charPoint = 0;
        returnData[charPoint++] = size;
        returnData[charPoint++] = size;
        for (int i = 0; i < size; i++){
            for (int j = 0; j < size; j++){
                unsigned long dtr = (((const unsigned long*) data)[i + j * size]);
                returnData[i + j * size + 2] = (dtr >> 48) | (dtr << ((sizeof(unsigned long)*8) - 48));
            }
        }
        return returnData;
    }
    ImageInput::~ImageInput() {
        stbi_image_free(data);
    }
}
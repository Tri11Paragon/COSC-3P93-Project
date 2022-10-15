/*
 * Created by Brett Terpstra 6920201 on 14/10/22.
 * Copyright (c) Brett Terpstra 2022 All Rights Reserved
 */
#include "image/image.h"

namespace Raytracing {

    Image::Image(int width, int height): width(width), height(height) {
        pixelData = new vec4[width * height];
    }

    Image::Image(const Image &image): width(image.width), height(image.height) {
        pixelData = new vec4[image.width * image.height];
        for (int i = 0; i < image.width; i++){
            for (int j = 0; j < image.height; j++){
                this->setPixelColor(i, j, image.pixelData[i * image.width + j]);
            }
        }
    }

    int Image::getPixelR(int x, int y) {
        // values are stored as a floating point number [0, 1)
        // but most formats want an int [0, 255]
        return (int) (255.999 * getPixelColor(x, y).r());
    }

    int Image::getPixelG(int x, int y) {
        return (int) (255.999 * getPixelColor(x, y).g());
    }

    int Image::getPixelB(int x, int y) {
        return (int) (255.999 * getPixelColor(x, y).b());
    }

    int Image::getPixelA(int x, int y) {
        return (int) (255.999 * getPixelColor(x, y).a());
    }

    Image::~Image() {
        delete[](pixelData);
    }
}
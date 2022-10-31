/*
 * Created by Brett Terpstra 6920201 on 14/10/22.
 * Copyright (c) Brett Terpstra 2022 All Rights Reserved
 */

#ifndef STEP_2_IMAGE_H
#define STEP_2_IMAGE_H

#include "engine/util/std.h"
#include "engine/math/vectors.h"

namespace Raytracing {

    // glorified structure to store our image data.
    class Image {
        private:
            unsigned long width;
            unsigned long height;
            unsigned long _width;
            unsigned long _height;
            Vec4 *pixelData;
        public:
            Image(unsigned long width, unsigned long height);
            Image(const Image &image);
            Image(const Image&& image) = delete;

            inline void setPixelColor(unsigned long x, unsigned long y, const Vec4 &color) {
                pixelData[(x * height) + y] = color;
            }

            [[nodiscard]] inline Vec4 getPixelColor(unsigned long x, unsigned long y) const {
                return pixelData[(x * height) + y];
            }

            [[nodiscard]] int getPixelR(int x, int y) const;
            [[nodiscard]] int getPixelG(int x, int y) const;
            [[nodiscard]] int getPixelB(int x, int y) const;
            [[nodiscard]] int getPixelA(int x, int y) const;

            [[nodiscard]] inline int getWidth() const { return int(width); }

            [[nodiscard]] inline int getHeight() const { return int(height); }

            ~Image();
    };

    class ImageInput {
        private:
            int width, height, channels;
            unsigned char* data;
        public:
            explicit ImageInput(const std::string& image);
            unsigned long* getImageAsIconBuffer();
            unsigned char* getImageData(){return data;}
            [[nodiscard]] int getImageSize() const{return width;}
            ~ImageInput();
    };
    
    // image writer class used to output the image to a file.
    class ImageOutput {
        private:
            Image image;
        public:
            explicit ImageOutput(const Image& image): image(image) {}
            virtual void write(const std::string& file, const  std::string& formatExtension);
    };

}

#endif //STEP_2_IMAGE_H

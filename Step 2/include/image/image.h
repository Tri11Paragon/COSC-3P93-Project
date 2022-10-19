/*
 * Created by Brett Terpstra 6920201 on 14/10/22.
 * Copyright (c) Brett Terpstra 2022 All Rights Reserved
 */

#ifndef STEP_2_IMAGE_H
#define STEP_2_IMAGE_H

#include "util/std.h"
#include "math/vectors.h"

namespace Raytracing {

    class Image {
        private:
            int width;
            int height;
            Vec4 *pixelData;
        public:
            Image(int width, int height);
            Image(const Image &image);
            Image(const Image&& image) = delete;

            inline void setPixelColor(int x, int y, const Vec4 &color) {
                pixelData[(x * height) + y] = color;
            }

            [[nodiscard]] inline Vec4 getPixelColor(int x, int y) const {
                return pixelData[(x * height) + y];
            }

            [[nodiscard]] int getPixelR(int x, int y) const;
            [[nodiscard]] int getPixelG(int x, int y) const;
            [[nodiscard]] int getPixelB(int x, int y) const;
            [[nodiscard]] int getPixelA(int x, int y) const;

            [[nodiscard]] inline int getWidth() const { return width; }

            [[nodiscard]] inline int getHeight() const { return height; }

            ~Image();
    };

    class ImageOutput {
        private:
            Image image;
        public:
            explicit ImageOutput(const Image& image): image(image) {}
            virtual void write(const std::string& file, const  std::string& formatExtension);
    };

}

#endif //STEP_2_IMAGE_H

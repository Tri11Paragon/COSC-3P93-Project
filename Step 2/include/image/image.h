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
            vec4 *pixelData;
        public:
            Image(int width, int height);
            Image(const Image &image);

            inline void setPixelColor(int x, int y, const vec4 &color) {
                pixelData[(x * height) + y] = color;
            }

            [[nodiscard]] inline vec4 getPixelColor(int x, int y) const {
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
            const Image& image;
        public:
            explicit ImageOutput(const Image& image): image(image) {}
            virtual void write(const std::string& file, const  std::string& formatExtension);
    };

}

#endif //STEP_2_IMAGE_H

/*
 * Created by Brett Terpstra 6920201 on 14/10/22.
 * Copyright (c) Brett Terpstra 2022 All Rights Reserved
 */

#ifndef STEP_2_IMAGE_H
#define STEP_2_IMAGE_H

#include "util/std.h"
#include "math/vectors.h"

namespace Raytracing {

    class Image{
    private:
        int width;
        int height;
        vec4* pixelData;
    public:
        Image(int width, int height);
        Image(const Image& image);

        inline void setPixelColor(int x, int y, const vec4& color){
            pixelData[x * width + y] = color;
        }
        inline vec4 getPixelColor(int x, int y){
            return pixelData[x * width + y];
        }
        int getPixelR(int x, int y);
        int getPixelG(int x, int y);
        int getPixelB(int x, int y);
        int getPixelA(int x, int y);

        [[nodiscard]] inline int getWidth() const {return width;}
        [[nodiscard]] inline int getHeight() const {return height;}

        ~Image();
    };

    class ImageOutput {
    private:
        
    };

}

#endif //STEP_2_IMAGE_H

/*
 * Created by Brett Terpstra 6920201 on 14/10/22.
 * Copyright (c) Brett Terpstra 2022 All Rights Reserved
 */

#ifndef STEP_2_IMAGE_H
#define STEP_2_IMAGE_H

#include "engine/util/std.h"
#include "engine/math/vectors.h"

namespace Raytracing {
    
    class Image {
        private:
            unsigned long width;
            unsigned long height;
            unsigned long _width;
            unsigned long _height;
            Vec4* pixelData;
            bool m_modified = false;
        public:
            Image(unsigned long width, unsigned long height);
            
            Image(const Image& image);
            
            Image(const Image&& image) = delete;
            
            /**
             * Converts the image to a array of doubles, the pixels are packed vectors of order (x,y,z,w)
             */
            std::vector<double> toArray();
            
            /**
             * Loads the pixel data from the double array, ignoring values which where not modified.
             * It would make more sense to send only modified data however it is much easier not to.
             * @param array array to load from
             * @param size size of the array
             * @param id unused
             */
            void fromArray(double* array, int size, int id);
            
            inline void setPixelColor(unsigned long x, unsigned long y, const Vec4& color) {
                m_modified = true;
                pixelData[(x * height) + y] = Vec4{color.r(), color.g(), color.b(), 1.0};
            }
            
            [[nodiscard]] inline Vec4 getPixelColor(unsigned long x, unsigned long y) const {
                return pixelData[(x * height) + y];
            }
            
            [[nodiscard]] inline int getPixelR(int x, int y) const {
                return int(255.0 * getPixelColor(x, y).r());
            };
            
            [[nodiscard]] inline int getPixelG(int x, int y) const {
                return int(255.0 * getPixelColor(x, y).g());
            };
            
            [[nodiscard]] inline int getPixelB(int x, int y) const {
                return int(255.0 * getPixelColor(x, y).b());
            }
            
            [[nodiscard]] inline int getPixelA(int x, int y) const {
                return int(255.0 * getPixelColor(x, y).a());
            }
            
            [[nodiscard]] inline int getWidth() const { return int(width); }
            
            [[nodiscard]] inline int getHeight() const { return int(height); }
            
            [[nodiscard]] inline bool modified() const { return m_modified; }
            
            ~Image();
    };
    
    class ImageInput {
        private:
            int width, height, channels;
            unsigned char* data;
        public:
            explicit ImageInput(const std::string& image);
            
            /**
             * Loads the image as a buffer for GUI icons
             * @return pointer that must be manually freed.
             */
            unsigned long* getImageAsIconBuffer();
            
            unsigned char* getImageData() { return data; }
            
            [[nodiscard]] int getImageSize() const { return width; }
            
            ~ImageInput();
    };
    
    // image writer class used to output the image to a file.
    class ImageOutput {
        private:
            const Image& image;
        public:
            explicit ImageOutput(const Image& image): image(image) {}
            
            /**
             * Writes the image stored in this class
             * @param file file to write to
             * @param formatExtension .png / .jpg etc
             */
            virtual void write(const std::string& file, const std::string& formatExtension);
    };
    
}

#endif //STEP_2_IMAGE_H

/*
 * Created by Brett Terpstra 6920201 on 03/12/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */

#ifndef STEP_3_MEMORY_UTIL_H
#define STEP_3_MEMORY_UTIL_H

#include <engine/util/std.h>

#define ENDIAN_FLIP

namespace Raytracing {
    
    class MemoryConvert {
        private:
        public:
            /**
             * returns the bytes from the type T ordered based on if ENDIAN_FLIP is defined or not
             */
            template<typename T>
            inline static std::vector<unsigned char> getBytes(T t) {
                std::vector<unsigned char> bytes;
                auto* asBytes = reinterpret_cast<unsigned char*>(&t);
#ifdef ENDIAN_FLIP
                for (int i = 0; i < sizeof(T); i++)
                    bytes.push_back(asBytes[i]);
#else
                for (int i = 0; i < sizeof(T); i++)
                    bytes.push_back(asBytes[sizeof(T) - i]);
#endif
                return bytes;
            }
            
            /**
             * Note: this is used for the GPU
             * Converts the vector to single floating point format.
             * @param array array to write into
             * @param offset offset of where to start writing into. Will update this value as it moves through the vector
             * @param vec the vector to write
             */
            inline static void writeVectorBytes(unsigned char* array, size_t& offset, const Vec4& vec) {
                auto x = getBytes((float) vec.x());
                auto y = getBytes((float) vec.y());
                auto z = getBytes((float) vec.z());
                auto w = getBytes((float) vec.w());
                // write the bytes as a packed vector.
                for (auto b: x)
                    array[offset++] = b;
                for (auto b: y)
                    array[offset++] = b;
                for (auto b: z)
                    array[offset++] = b;
                for (auto b: w)
                    array[offset++] = b;
            }
            
            /**
             * Note: this is used for the GPU.
             * @param array array to write into
             * @param offset offset of where to start writing into. Will update this value as it moves through the vector
             * @param integer the int to write
             */
            template<typename T>
            inline static void writeBytes(unsigned char* array, size_t& offset, const T t) {
                auto x = getBytes(t);
                for (auto b: x)
                    array[offset++] = b;
            }
    };
}

#endif //STEP_3_MEMORY_UTIL_H

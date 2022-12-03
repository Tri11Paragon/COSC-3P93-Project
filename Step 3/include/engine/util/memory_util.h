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
             * returns the bytes from the double ordered based on if ENDIAN_FLIP is defined or not
             */
            inline static std::vector<unsigned char> getDoubleBytes(double d) {
                std::vector<unsigned char> bytes;
                auto* doubleAsBytes = reinterpret_cast<unsigned char*>(&d);
#ifdef ENDIAN_FLIP
                for (int i = 0; i < sizeof(double); i++)
                    bytes.push_back(doubleAsBytes[i]);
#else
                for (int i = 0; i < sizeof(double); i++)
                    bytes.push_back(doubleAsBytes[sizeof(double) - i]);
#endif
                return bytes;
            }
            
            /**
             * Note: this is used for the GPU and it is assumed that you are using double precision values.
             * Anything else is undefined. (Should be fine though since upcasting is fine)
             * @param array array to write into
             * @param offset offset of where to start writing into. Will update this value as it moves through the vector
             * @param vec the vector to write
             */
            inline static void writeVectorBytes(unsigned char* array, size_t& offset, const Vec4& vec) {
                auto x = getDoubleBytes(vec.x());
                auto y = getDoubleBytes(vec.y());
                auto z = getDoubleBytes(vec.z());
                auto w = getDoubleBytes(vec.w());
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
    };
}

#endif //STEP_3_MEMORY_UTIL_H

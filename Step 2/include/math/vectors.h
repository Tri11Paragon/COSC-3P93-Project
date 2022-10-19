/*
 * Created by Brett Terpstra 6920201 on 14/10/22.
 * Copyright (c) Brett Terpstra 2022 All Rights Reserved
 */

#ifndef STEP_2_VECTORS_H
#define STEP_2_VECTORS_H

// this is a big TODO: add AVX2 instructions to vector class, AVX512 isn't supported on my CPU.
#define USE_SIMD_CPU

#ifdef USE_SIMD_CPU
    #include <immintrin.h>
#endif

#include <cmath>
#include "util/std.h"

namespace Raytracing {

    // when running on the CPU it's fine to be a double
    // Your CPU may be faster with floats.
    // but if we move to the GPU it has to be a float.
    // since GPUs generally are far more optimized for floats
    // If using AVX or other SIMD instructions it should be double, only to fit into 256bits.

    #ifdef USE_SIMD_CPU
        // don't change this.
        typedef double PRECISION_TYPE;

        class Vec4 {
            private:
                __m128 VectorValues;
            public:
                Vec4(): VectorValues{_mm256_set_pd(0.0, 0.0, 0.0, 0.0)} {}
                Vec4(PRECISION_TYPE x, PRECISION_TYPE y, PRECISION_TYPE z): VectorValues{x, y, z, 0} {}
                Vec4(PRECISION_TYPE x, PRECISION_TYPE y, PRECISION_TYPE z, PRECISION_TYPE w): VectorValues{x, y, z, w} {}
                Vec4(const Vec4& vec): VectorValues{vec.x(), vec.y(), vec.z(), vec.w()} {}
        }
    #elif
        // change this if you want
        typedef double PRECISION_TYPE;

        class Vec4 {
            private:
                union xType {
                    PRECISION_TYPE x;
                    PRECISION_TYPE r;
                };
                union yType {
                    PRECISION_TYPE y;
                    PRECISION_TYPE g;
                };
                union zType {
                    PRECISION_TYPE z;
                    PRECISION_TYPE b;
                };
                union wType {
                    PRECISION_TYPE w;
                    PRECISION_TYPE a;
                };

                struct valueType {
                    xType v1;
                    yType v2;
                    zType v3;
                    wType v4;
                };
                // isn't much of a reason to do it this way
                // beyond I wanted an explicit immutable vector type of length 4
                // that could be used as both x,y,z + w? and rgba
                // it's unlikely that we'll need to use the w component
                // but it helps better line up with the GPU
                // and floating point units (especially on GPUs) tend to be aligned to 4*sizeof(float)
                valueType value;
            public:
                Vec4(): value{0, 0, 0, 0} {}
                Vec4(PRECISION_TYPE x, PRECISION_TYPE y, PRECISION_TYPE z): value{x, y, z, 0} {}
                Vec4(PRECISION_TYPE x, PRECISION_TYPE y, PRECISION_TYPE z, PRECISION_TYPE w): value{x, y, z, w} {}
                Vec4(const Vec4& vec): value{vec.x(), vec.y(), vec.z(), vec.w()} {}


                // most of the modern c++ here is because clang tidy was annoying me
                [[nodiscard]] inline PRECISION_TYPE x() const { return value.v1.x; }

                [[nodiscard]] inline PRECISION_TYPE y() const { return value.v2.y; }

                [[nodiscard]] inline PRECISION_TYPE z() const { return value.v3.z; }

                [[nodiscard]] inline PRECISION_TYPE w() const { return value.v4.w; }

                [[nodiscard]] inline PRECISION_TYPE r() const { return value.v1.r; }

                [[nodiscard]] inline PRECISION_TYPE g() const { return value.v2.g; }

                [[nodiscard]] inline PRECISION_TYPE b() const { return value.v3.b; }

                [[nodiscard]] inline PRECISION_TYPE a() const { return value.v4.a; }

                // negation operator
                Vec4 operator-() const { return {-x(), -y(), -z(), -w()}; }

                [[nodiscard]] inline PRECISION_TYPE magnitude() const {
                    return sqrt(lengthSquared());
                }

                [[nodiscard]] inline PRECISION_TYPE lengthSquared() const {
                    return x() * x() + y() * y() + z() * z() + w() * w();
                }

                // returns the unit-vector.
                [[nodiscard]] inline Vec4 normalize() const {
                    PRECISION_TYPE mag = magnitude();
                    return {x() / mag, y() / mag, z() / mag, w() / mag};
                }

                // add operator before the vec returns the magnitude
                PRECISION_TYPE operator+() const {
                    return magnitude();
                }

                // preforms the dot product of left * right
                static inline PRECISION_TYPE dot(const Vec4& left, const Vec4& right) {
                    return left.x() * right.x()
                        + left.y() * right.y()
                        + left.z() * right.z();
                }

                // preforms the cross product of left X right
                // since a general solution to the cross product doesn't exist in 4d
                // we are going to ignore the w.
                static inline Vec4 cross(const Vec4& left, const Vec4& right) {
                    return {left.y() * right.z() - left.z() * right.y(),
                            left.z() * right.x() - left.x() * right.z(),
                            left.x() * right.y() - left.y() * right.x()};
                }

        };

    // Utility Functions

        // useful for printing out the vector to stdout
        inline std::ostream& operator<<(std::ostream& out, const Vec4& v) {
            return out << "Vec4{" << v.x() << ", " << v.y() << ", " << v.z() << ", " << v.w() << "} ";
        }

        // adds the two vectors left and right
        inline Vec4 operator+(const Vec4& left, const Vec4& right) {
            return {left.x() + right.x(), left.y() + right.y(), left.z() + right.z(), left.w() + right.w()};
        }

        // subtracts the right vector from the left.
        inline Vec4 operator-(const Vec4& left, const Vec4& right) {
            return {left.x() - right.x(), left.y() - right.y(), left.z() - right.z(), left.w() - right.w()};
        }

        // multiples the left with the right
        inline Vec4 operator*(const Vec4& left, const Vec4& right) {
            return {left.x() * right.x(), left.y() * right.y(), left.z() * right.z(), left.w() * right.w()};
        }

        // divides each element individually
        inline Vec4 operator/(const Vec4& left, const Vec4& right) {
            return {left.x() / right.x(), left.y() / right.y(), left.z() / right.z(), left.w() / right.w()};
        }

        // multiplies the const c with each element in the vector v
        inline Vec4 operator*(const PRECISION_TYPE c, const Vec4& v) {
            return {c * v.x(), c * v.y(), c * v.z(), c * v.w()};
        }

        // same as above but for right sided constants
        inline Vec4 operator*(const Vec4& v, PRECISION_TYPE c) {
            return c * v;
        }

        // divides the vector by the constant c
        inline Vec4 operator/(const Vec4& v, PRECISION_TYPE c) {
            return {v.x() / c, v.y() / c, v.z() / c, v.w() / c};
        }

        // divides each element in the vector by over the constant
        inline Vec4 operator/(PRECISION_TYPE c, const Vec4& v) {
            return {c / v.x(), c / v.y(), c / v.z(), c / v.w()};
        }
    #endif

    class Ray {
        private:
            // the starting point for our ray
            Vec4 start;
            // and the direction it is currently traveling
            Vec4 direction;
            Vec4 inverseDirection;
        public:
            Ray(const Vec4& start, const Vec4& direction): start(start), direction(direction), inverseDirection(1 / direction) {}

            [[nodiscard]] Vec4 getStartingPoint() const { return start; }

            [[nodiscard]] Vec4 getDirection() const { return direction; }

            // not always needed, but it's good to not have to calculate the inverse inside the intersection
            // as that would be very every AABB, and that is expensive
            [[nodiscard]] Vec4 getInverseDirection() const {return inverseDirection; }

            // returns a point along the ray, extended away from start by the length.
            [[nodiscard]] inline Vec4 along(PRECISION_TYPE length) const { return start + length * direction; }

    };

    inline std::ostream& operator<<(std::ostream& out, const Ray& v) {
        return out << "Ray{" << v.getStartingPoint() << " " << v.getDirection() << "} ";
    }

}

#endif //STEP_2_VECTORS_H

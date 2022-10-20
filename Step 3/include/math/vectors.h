/*
 * Created by Brett Terpstra 6920201 on 14/10/22.
 * Copyright (c) Brett Terpstra 2022 All Rights Reserved
 */

#ifndef STEP_2_VECTORS_H
#define STEP_2_VECTORS_H

// AVX512 isn't supported on my CPU. We will use AVX2 since it is supported by most modern CPUs
#define USE_SIMD_CPU

#ifdef USE_SIMD_CPU
    
    #include <immintrin.h>

#endif

#include <cmath>
#include <util/std.h>

namespace Raytracing {
    
    // when running on the CPU it's fine to be a double
    // Your CPU may be faster with floats.
    // but if we move to the GPU it has to be a float.
    // since GPUs generally are far more optimized for floats
    // If using AVX or other SIMD instructions it should be double, only to fit into 256bits.
    // TODO would be to add support for 128bit AVX vectors.
    
    #ifdef USE_SIMD_CPU
    // don't change this. (working on a float version)
    typedef double PRECISION_TYPE;
    
    union AVXConvert {
        struct {
            double _x, _y, _z, _w;
        };
        __m256d avxData;
    };
    
    class Vec4 {
        private:
            // makes it easy to convert between AVX and double data types.
            union {
                struct {
                    PRECISION_TYPE _x{}, _y{}, _z{}, _w{};
                    //PRECISION_TYPE _w, _z, _y, _x;
                };
                __m256d avxData;
            };
            
            // finally a use for friend!
            friend Vec4 operator+(const Vec4& left, const Vec4& right);
            friend Vec4 operator-(const Vec4& left, const Vec4& right);
            friend Vec4 operator*(const Vec4& left, const Vec4& right);
            friend Vec4 operator/(const Vec4& left, const Vec4& right);
        public:
            
            Vec4(): avxData(_mm256_setzero_pd()) {}
            Vec4(const __m256d& data): avxData(data) {}
            Vec4(PRECISION_TYPE x, PRECISION_TYPE y, PRECISION_TYPE z): avxData(_mm256_setr_pd(x, y, z, 0.0)) {
                //tlog << x << ":" << _x << " " << y << ":" << _y << " " << z << ":" << _z << "\n";
            }
            Vec4(PRECISION_TYPE x, PRECISION_TYPE y, PRECISION_TYPE z, PRECISION_TYPE w): avxData(_mm256_setr_pd(x, y, z, w)) {
                //dlog << x << ":" << _x << " " << y << ":" << _y << " " << z << ":" << _z << "\n";
            }
            Vec4(const Vec4& vec): avxData(_mm256_setr_pd(vec.x(), vec.y(), vec.z(), vec.w())) {
                //ilog << vec.x() << ":" << _x << " " << vec.y() << ":" << _y << " " << vec.z() << ":" << _z << "\n";
            }
            
            // most of the modern c++ here is because clang tidy was annoying me
            [[nodiscard]] inline PRECISION_TYPE x() const { return _x; }
            
            [[nodiscard]] inline PRECISION_TYPE y() const { return _y; }
            
            [[nodiscard]] inline PRECISION_TYPE z() const { return _z; }
            
            [[nodiscard]] inline PRECISION_TYPE w() const { return _w; }
            
            [[nodiscard]] inline PRECISION_TYPE r() const { return _x; }
            
            [[nodiscard]] inline PRECISION_TYPE g() const { return _y; }
            
            [[nodiscard]] inline PRECISION_TYPE b() const { return _z; }
            
            [[nodiscard]] inline PRECISION_TYPE a() const { return _w; }
            
            // negation operator
            Vec4 operator-() const { return {-x(), -y(), -z(), -w()}; }
        
            [[nodiscard]] inline PRECISION_TYPE magnitude() const {
                return sqrt(lengthSquared());
            }
        
            [[nodiscard]] inline PRECISION_TYPE lengthSquared() const {
                __m256d multiplied = _mm256_mul_pd(avxData, avxData);
                // horizontal add. element 0 and 2 (or 1 and 3) contain the results which we must scalar add.
                __m256d sum = _mm256_hadd_pd(multiplied, multiplied);
                AVXConvert conv;
                conv.avxData = sum;
                return conv._x + conv._z;
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
                // multiply the elements of the vectors
                __m256d mul = _mm256_mul_pd(left.avxData, right.avxData);
                // horizontal add. element 0 and 2 (or 1 and 3) contain the results which we must scalar add.
                __m256d sum = _mm256_hadd_pd(mul, mul);
                AVXConvert conv {};
                conv.avxData = sum;
                // boom! dot product. much easier than cross
                return conv._x + conv._z;
            }
        
            // preforms the cross product of left X right
            // since a general solution to the cross product doesn't exist in 4d
            // we are going to ignore the w.
            static inline Vec4 cross(const Vec4& left, const Vec4& right) {
                // shuffle left values for alignment with the cross algorithm
                // (read the shuffle selector from right to left) takes the y and places it in the first element of the resultant vector
                // takes the z and places it in the second element of the vector
                // takes the x element and places it in the 3rd element of the vector
                // and then the w element in the last element of the vector
                // creating the alignment {left.y(), left.z(), left.x(), left.w()} (as seen in the cross algorithm
                __m256d leftLeftShuffle = _mm256_permute4x64_pd(left.avxData, _MM_SHUFFLE(3,0,2,1));
                // same thing but produces {right.z(), right.x(), right.y(), right.w()}
                __m256d rightLeftShuffle = _mm256_permute4x64_pd(right.avxData, _MM_SHUFFLE(3,1,0,2));
                // now we have to do the right side multiplications
                // {left.z(), left.x(), left.y(), left.w()}
                __m256d leftRightShuffle = _mm256_permute4x64_pd(left.avxData, _MM_SHUFFLE(3,1,0,2));
                // {right.y(), right.z(), right.x(), right.w()}
                __m256d rightRightShuffle = _mm256_permute4x64_pd(right.avxData, _MM_SHUFFLE(3,0,2,1));
                // multiply to do the first step of the cross process
                __m256d multiLeft = _mm256_mul_pd(leftLeftShuffle, rightLeftShuffle);
                // multiply the right sides of the subtraction sign
                __m256d multiRight = _mm256_mul_pd(leftRightShuffle, rightRightShuffle);
                // then subtract to produce the cross product
                __m256d subs = _mm256_sub_pd(multiLeft, multiRight);
                // yes this looks a lot more complicated, but it should be faster!
                /*auto b = Vec4{left.y() * right.z() - left.z() * right.y(),
                          left.z() * right.x() - left.x() * right.z(),
                          left.x() * right.y() - left.y() * right.x()};
                tlog << b._x << " " << b._y << " " << b._z << "\n";
                tlog << conv._x << " " << conv._y << " " << conv._z << "\n\n";*/
                return {subs};
            }
    };
    
    // adds the two vectors left and right
    inline Vec4 operator+(const Vec4& left, const Vec4& right) {
        __m256d added = _mm256_add_pd(left.avxData, right.avxData);
        return {added};
    }
    
    // subtracts the right vector from the left.
    inline Vec4 operator-(const Vec4& left, const Vec4& right) {
        __m256d subbed = _mm256_sub_pd(left.avxData, right.avxData);
        return {subbed};
    }
    
    // multiples the left with the right
    inline Vec4 operator*(const Vec4& left, const Vec4& right) {
        //dlog << left._x << " " << left._y << " " << left._z << " " << left._w << "\n";
        //dlog << right._x << " " << right._y << " " << right._z << " " << right._w << "\n";
        __m256d multiplied = _mm256_mul_pd(left.avxData, right.avxData);
        //dlog << conv._x << " " << conv._y << " " << conv._z << " " << conv._w << "\n\n";
        return {multiplied};
    }
    
    // divides each element individually
    inline Vec4 operator/(const Vec4& left, const Vec4& right) {
        __m256d dived = _mm256_div_pd(left.avxData, right.avxData);
        return {dived};
    }
    
    #else
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
            // it's unlikely that we'll need to use the w component
            // but it helps better line up with the GPU and other SIMD type instructions, like what's above.
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
    
    #endif
    
    // none of these can be vectorized with AVX instructions
    
    // useful for printing out the vector to stdout
    inline std::ostream& operator<<(std::ostream& out, const Vec4& v) {
        return out << "Vec4{" << v.x() << ", " << v.y() << ", " << v.z() << ", " << v.w() << "} ";
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
            [[nodiscard]] Vec4 getInverseDirection() const { return inverseDirection; }
            
            // returns a point along the ray, extended away from start by the length.
            [[nodiscard]] inline Vec4 along(PRECISION_TYPE length) const { return start + length * direction; }
        
    };
    
    inline std::ostream& operator<<(std::ostream& out, const Ray& v) {
        return out << "Ray{" << v.getStartingPoint() << " " << v.getDirection() << "} ";
    }
    
}

#endif //STEP_2_VECTORS_H

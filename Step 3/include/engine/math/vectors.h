/*
 * Created by Brett Terpstra 6920201 on 14/10/22.
 * Copyright (c) Brett Terpstra 2022 All Rights Reserved
 */

#ifndef STEP_2_VECTORS_H
#define STEP_2_VECTORS_H

// AVX512 isn't supported on my CPU. We will use AVX2 since it is supported by most modern CPUs
#include "config.h"
//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtc/type_ptr.hpp>

// I have tested this and when in release mode the O3 optimizations are capable of creating
// far better auto-vectorized results. See the table below for more info.
// but in debug mode using the AVX instructions is far better. As they say, never try to out optimize the compiler - you'll lose.

// in debug mode:
// multiplication
// 2174.43ms normal
// 1483.04ms avx
// division
// 2282.44ms normal
// 1627ms avx
// addition
// 2119.4ms normal
// 1495.77ms avx
// dot
// 1447.9ms normal
// 1088.5ms avx
// cross
// 2840.69ms normal
// 2543.66ms avx

// with release mode
// cross
// 244.144ms normal
// 283.516ms avx
// dot
// 239.759ms normal
// 385.583ms avx
// mul
// 70.9977ms normal
// 286.656ms avx

#ifdef COMPILER_DEBUG_ENABLED
    #define USE_SIMD_CPU
#endif

#ifdef USE_SIMD_CPU
    
    #include <immintrin.h>

#endif

//#include <cmath>
#include "engine/util/std.h"

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
            friend Vec4 operator*(PRECISION_TYPE c, const Vec4& v);
            friend Vec4 operator*(const Vec4& v, PRECISION_TYPE c);
            friend Vec4 operator/(const Vec4& v, PRECISION_TYPE c);
            friend Vec4 operator/(PRECISION_TYPE c, const Vec4& v);
        public:
            
            Vec4(): avxData(_mm256_setzero_pd()) {}
            explicit Vec4(const __m256d& data): avxData(data) {}
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
            
            static inline __m256d getVecFromValue(PRECISION_TYPE c) {
                return _mm256_set1_pd(c);
            }
            
            // negation operator
            Vec4 operator-() const {
                return Vec4{_mm256_mul_pd(getVecFromValue(-1), this->avxData)};
            }
        
            [[nodiscard]] inline PRECISION_TYPE magnitude() const {
                return sqrt(lengthSquared());
            }
        
            [[nodiscard]] inline PRECISION_TYPE lengthSquared() const {
                return dot(*this, *this);
            }
        
            // returns the unit-vector.
            [[nodiscard]] inline Vec4 normalize() const {
                return Vec4{_mm256_div_pd(avxData, getVecFromValue(magnitude()))};
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
                return Vec4{subs};
            }
    };
    
    // adds the two vectors left and right
    inline Vec4 operator+(const Vec4& left, const Vec4& right) {
        return Vec4{_mm256_add_pd(left.avxData, right.avxData)};
    }
    
    // subtracts the right vector from the left.
    inline Vec4 operator-(const Vec4& left, const Vec4& right) {
        return Vec4{_mm256_sub_pd(left.avxData, right.avxData)};
    }
    
    // multiples the left with the right
    inline Vec4 operator*(const Vec4& left, const Vec4& right) {
        //dlog << left._x << " " << left._y << " " << left._z << " " << left._w << "\n";
        //dlog << right._x << " " << right._y << " " << right._z << " " << right._w << "\n";
        //dlog << conv._x << " " << conv._y << " " << conv._z << " " << conv._w << "\n\n";
        return Vec4{_mm256_mul_pd(left.avxData, right.avxData)};
    }
    
    // divides each element individually
    inline Vec4 operator/(const Vec4& left, const Vec4& right) {
        return Vec4{_mm256_div_pd(left.avxData, right.avxData)};
    }
    
    // multiplies the const c with each element in the vector v
    inline Vec4 operator*(PRECISION_TYPE c, const Vec4& v) {
        return Vec4{_mm256_mul_pd(Vec4::getVecFromValue(c), v.avxData)};
    }
    
    // same as above but for right sided constants
    inline Vec4 operator*(const Vec4& v, PRECISION_TYPE c) {
        return Vec4{_mm256_mul_pd(v.avxData, Vec4::getVecFromValue(c))};
    }
    
    // divides the vector by the constant c
    inline Vec4 operator/(const Vec4& v, PRECISION_TYPE c) {
        return Vec4{_mm256_div_pd(v.avxData, Vec4::getVecFromValue(c))};
    }
    
    // divides each element in the vector by over the constant
    inline Vec4 operator/(PRECISION_TYPE c, const Vec4& v) {
        return Vec4{_mm256_div_pd(Vec4::getVecFromValue(c), v.avxData)};
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
    
    // adds all vector axis with a constant
    inline Vec4 operator+(const Vec4& left, const PRECISION_TYPE right) {
        return {left.x() + right, left.y() + right, left.z() + right, left.w() + right};
    }
    
    // checks if the 3 major axis on the left are less than the right
    inline bool operator<(const Vec4& left, const Vec4& right) {
        return left.x() < right.x() && left.y() < right.y() && left.z() < right.z();
    }
    
    // checks if the 3 major axis on the left are greater than the right
    inline bool operator>(const Vec4& left, const Vec4& right) {
        return left.x() > right.x() && left.y() > right.y() && left.z() > right.z();
    }
    
    // checks if the 3 major axis on the left are less equal than the right
    inline bool operator<=(const Vec4& left, const Vec4& right) {
        return left.x() <= right.x() && left.y() <= right.y() && left.z() <= right.z() && left.w() <= right.w();
    }
    
    // checks if all major axis on the left are greater equal than the right
    inline bool operator>=(const Vec4& left, const Vec4& right) {
        return left.x() >= right.x() && left.y() >= right.y() && left.z() >= right.z() && left.w() >= right.w();
    }
    
    // subtracts the right vector from the left.
    inline Vec4 operator-(const Vec4& left, const Vec4& right) {
        return {left.x() - right.x(), left.y() - right.y(), left.z() - right.z(), left.w() - right.w()};
    }
    
    // subtracts all vector axis with a constant
    inline Vec4 operator-(const Vec4& left, const PRECISION_TYPE right) {
        return {left.x() - right, left.y() - right, left.z() - right, left.w() - right};
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
        return {v.x() * c, v.y() * c, v.z() * c, v.w() * c};
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
    
    // none of these can be vectorized with AVX instructions
    
    // useful for printing out the vector to stdout
    inline std::ostream& operator<<(std::ostream& out, const Vec4& v) {
        return out << "Vec4{" << v.x() << ", " << v.y() << ", " << v.z() << ", " << v.w() << "} ";
    }
    
    inline bool operator==(const Vec4& left, const Vec4& right) {
        return left.x() == right.x() && left.y() == right.y() && left.z() == right.z() && left.w() == right.w();
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

#ifdef USE_SIMD_CPU

#else

#endif
    
    // only float supported because GPUs don't like doubles
    // well they do but there isn't much of a reason to use them since this is for opengl
    class Mat4x4 {
        protected:
            // 4x4 = 16
            union dataType {
                float single[16];
                float dim[4][4];
            };
            dataType data {};
            friend Mat4x4 operator+(const Mat4x4& left, const Mat4x4& right);
            friend Mat4x4 operator-(const Mat4x4& left, const Mat4x4& right);
            friend Mat4x4 operator*(const Mat4x4& left, const Mat4x4& right);
            friend Mat4x4 operator*(float c, const Mat4x4& v);
            friend Mat4x4 operator*(const Mat4x4& v, float c);
            friend Mat4x4 operator/(const Mat4x4& v, float c);
            friend Mat4x4 operator/(float c, const Mat4x4& v);
        public:
            Mat4x4() {
                for (float & i : data.single) {
                    i = 0;
                }
                // set identity matrix default
                m00(1);
                m11(1);
                m22(1);
                m33(1);
            }
            /*explicit Mat4x4(glm::mat4x4 mat) {
                m00(mat[0][0]);
                m01(mat[1][0]);
                m02(mat[2][0]);
                m03(mat[3][0]);
    
                m10(mat[0][1]);
                m11(mat[1][1]);
                m12(mat[2][1]);
                m13(mat[3][1]);
    
                m20(mat[0][2]);
                m21(mat[1][2]);
                m22(mat[2][2]);
                m23(mat[3][2]);
    
                m30(mat[0][3]);
                m31(mat[1][3]);
                m32(mat[2][3]);
                m33(mat[3][3]);
            }*/
            Mat4x4(const Mat4x4& mat) {
                for (int i = 0; i < 16; i++) {
                    data.single[i] = mat.data.single[i];
                }
            }
            explicit Mat4x4(const float dat[16]) {
                for (int i = 0; i < 16; i++) {
                    data.single[i] = dat[i];
                }
            }
            
            inline Mat4x4& translate(float x, float y, float z) {
                m03(x);
                m13(y);
                m23(z);
                return *this;
            }
        
            inline Mat4x4& translate(const Vec4& vec) {
                m03(float(vec.x()));
                m13(float(vec.y()));
                m23(float(vec.z()));
                return *this;
            }
            
            inline Mat4x4& scale(float x, float y, float z) {
                m00(x);
                m11(y);
                m22(z);
                return *this;
            }
        
            inline Mat4x4& scale(const Vec4& vec) {
                m00(float(vec.x()));
                m11(float(vec.y()));
                m22(float(vec.z()));
                return *this;
            }
            
            float* ptr() {
                return data.single;
            }
            
            Mat4x4& transpose() {
                Mat4x4 copy {*this};
    
                m00(copy.m00());
                m01(copy.m10());
                m02(copy.m20());
                m03(copy.m30());
    
                m10(copy.m01());
                m11(copy.m11());
                m12(copy.m21());
                m13(copy.m31());
    
                m20(copy.m02());
                m21(copy.m12());
                m22(copy.m22());
                m23(copy.m32());
    
                m30(copy.m03());
                m31(copy.m13());
                m32(copy.m23());
                m33(copy.m33());
                
                return *this;
            }
            
            // Due to the conversion between the 2d array -> 1d array we must transpose the values.
            // the old system has been archived (commented) for future debugging
//            [[nodiscard]] inline float m00() const { return data.dim[0][0]; }
//            [[nodiscard]] inline float m10() const { return data.dim[1][0]; }
//            [[nodiscard]] inline float m20() const { return data.dim[2][0]; }
//            [[nodiscard]] inline float m30() const { return data.dim[3][0]; }
//            [[nodiscard]] inline float m01() const { return data.dim[0][1]; }
//            [[nodiscard]] inline float m11() const { return data.dim[1][1]; }
//            [[nodiscard]] inline float m21() const { return data.dim[2][1]; }
//            [[nodiscard]] inline float m31() const { return data.dim[3][1]; }
//            [[nodiscard]] inline float m02() const { return data.dim[0][2]; }
//            [[nodiscard]] inline float m12() const { return data.dim[1][2]; }
//            [[nodiscard]] inline float m22() const { return data.dim[2][2]; }
//            [[nodiscard]] inline float m32() const { return data.dim[3][2]; }
//            [[nodiscard]] inline float m03() const { return data.dim[0][3]; }
//            [[nodiscard]] inline float m13() const { return data.dim[1][3]; }
//            [[nodiscard]] inline float m23() const { return data.dim[2][3]; }
//            [[nodiscard]] inline float m33() const { return data.dim[3][3]; }
//            [[nodiscard]] inline float m(int i, int j) const { return data.dim[i][j]; };
//            inline float m00(float d) { return data.dim[0][0] = d; }
//            inline float m10(float d) { return data.dim[1][0] = d; }
//            inline float m20(float d) { return data.dim[2][0] = d; }
//            inline float m30(float d) { return data.dim[3][0] = d; }
//            inline float m01(float d) { return data.dim[0][1] = d; }
//            inline float m11(float d) { return data.dim[1][1] = d; }
//            inline float m21(float d) { return data.dim[2][1] = d; }
//            inline float m31(float d) { return data.dim[3][1] = d; }
//            inline float m02(float d) { return data.dim[0][2] = d; }
//            inline float m12(float d) { return data.dim[1][2] = d; }
//            inline float m22(float d) { return data.dim[2][2] = d; }
//            inline float m32(float d) { return data.dim[3][2] = d; }
//            inline float m03(float d) { return data.dim[0][3] = d; }
//            inline float m13(float d) { return data.dim[1][3] = d; }
//            inline float m23(float d) { return data.dim[2][3] = d; }
//            inline float m33(float d) { return data.dim[3][3] = d; }
            
            [[nodiscard]] inline float m00() const { return data.dim[0][0]; }
            [[nodiscard]] inline float m10() const { return data.dim[0][1]; }
            [[nodiscard]] inline float m20() const { return data.dim[0][2]; }
            [[nodiscard]] inline float m30() const { return data.dim[0][3]; }
            [[nodiscard]] inline float m01() const { return data.dim[1][0]; }
            [[nodiscard]] inline float m11() const { return data.dim[1][1]; }
            [[nodiscard]] inline float m21() const { return data.dim[1][2]; }
            [[nodiscard]] inline float m31() const { return data.dim[1][3]; }
            [[nodiscard]] inline float m02() const { return data.dim[2][0]; }
            [[nodiscard]] inline float m12() const { return data.dim[2][1]; }
            [[nodiscard]] inline float m22() const { return data.dim[2][2]; }
            [[nodiscard]] inline float m32() const { return data.dim[2][3]; }
            [[nodiscard]] inline float m03() const { return data.dim[3][0]; }
            [[nodiscard]] inline float m13() const { return data.dim[3][1]; }
            [[nodiscard]] inline float m23() const { return data.dim[3][2]; }
            [[nodiscard]] inline float m33() const { return data.dim[3][3]; }
            [[nodiscard]] inline float m(int i, int j) const { return data.dim[i][j]; };
            inline float m00(float d) { return data.dim[0][0] = d; }
            inline float m10(float d) { return data.dim[0][1] = d; }
            inline float m20(float d) { return data.dim[0][2] = d; }
            inline float m30(float d) { return data.dim[0][3] = d; }
            inline float m01(float d) { return data.dim[1][0] = d; }
            inline float m11(float d) { return data.dim[1][1] = d; }
            inline float m21(float d) { return data.dim[1][2] = d; }
            inline float m31(float d) { return data.dim[1][3] = d; }
            inline float m02(float d) { return data.dim[2][0] = d; }
            inline float m12(float d) { return data.dim[2][1] = d; }
            inline float m22(float d) { return data.dim[2][2] = d; }
            inline float m32(float d) { return data.dim[2][3] = d; }
            inline float m03(float d) { return data.dim[3][0] = d; }
            inline float m13(float d) { return data.dim[3][1] = d; }
            inline float m23(float d) { return data.dim[3][2] = d; }
            inline float m33(float d) { return data.dim[3][3] = d; }
            inline float m(int i, int j, float d) { return data.dim[i][j] = d; };
//            inline float* operator [](int _i) {
//                return data.dim[_i];
//            }
            
            [[nodiscard]] PRECISION_TYPE determinant() const {
                return m00() * (m11() * m22() * m33() + m12() * m23() * m31() + m13() * m21() * m32()
                                    - m31() * m22() * m13() - m32() * m23() * m11() - m33() * m21() * m12())
                        - m10() * (m01() * m22() * m33() + m02() * m23() * m31() + m03() * m21() * m32()
                                    - m31() * m32() * m03() - m32() * m23() * m01() - m33() * m21() * m02())
                        + m20() * (m01() * m12() * m33() + m02() * m13() * m31() + m03() * m11() * m32()
                                    - m31() * m12() * m03() - m32() * m13() * m01() - m33() * m11() * m02())
                        - m30() * (m01() * m12() * m23() + m02() * m13() * m21() + m03() * m11() * m22()
                                    - m21() * m12() * m03() - m22() * m13() * m01() - m23() * m11() * m02());
            }
    };
    
    // adds the two Mat4x4 left and right
    inline Mat4x4 operator+(const Mat4x4& left, const Mat4x4& right) {
        float data[16];
        for (int i = 0; i < 16; i++)
            data[i] = left.data.single[i] + right.data.single[i];
        return Mat4x4{data};
    }
    
    // subtracts the right Mat4x4 from the left.
    inline Mat4x4 operator-(const Mat4x4& left, const Mat4x4& right) {
        float data[16];
        for (int i = 0; i < 16; i++)
            data[i] = left.data.single[i] - right.data.single[i];
        return Mat4x4{data};
    }
    
    // since matrices are made identity by default, we need to create the result collector matrix without identity
    // otherwise the diagonal will be 1 off and cause weird results (see black screen issue)
    constexpr float emptyMatrix[16] = {0, 0, 0, 0,
                             0, 0, 0, 0,
                             0, 0, 0, 0,
                             0, 0, 0, 0};
    
    // multiples the left with the right
    inline Mat4x4 operator*(const Mat4x4& left, const Mat4x4& right) {
        Mat4x4 mat{emptyMatrix};
        
        // TODO: check avx with this??
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                for (int k = 0; k < 4; k++) {
                    mat.m(i, j, mat.m(i, j) + left.m(i, k) * right.m(k, j));
                }
            }
        }
        
        return mat;
    }
    
    // multiplies the const c with each element in the Mat4x4 v
    inline Mat4x4 operator*(float c, const Mat4x4& v) {
        Mat4x4 mat{};
        
        for (int i = 0; i < 16; i++) {
            mat.data.single[i] = c * v.data.single[i];
        }
        
        return mat;
    }
    
    // same as above but for right sided constants
    inline Mat4x4 operator*(const Mat4x4& v, float c) {
        Mat4x4 mat{};
    
        for (int i = 0; i < 16; i++) {
            mat.data.single[i] = v.data.single[i] * c;
        }
    
        return mat;
    }
    
    // divides the Mat4x4 by the constant c
    inline Mat4x4 operator/(const Mat4x4& v, float c) {
        Mat4x4 mat{};
    
        for (int i = 0; i < 16; i++) {
            mat.data.single[i] = v.data.single[i] / c;
        }
    
        return mat;
    }
    
    // divides each element in the Mat4x4 by over the constant
    inline Mat4x4 operator/(float c, const Mat4x4& v) {
        Mat4x4 mat{};
    
        for (int i = 0; i < 16; i++) {
            mat.data.single[i] = c / v.data.single[i];
        }
    
        return mat;
    }
    
    inline std::ostream& operator<<(std::ostream& out, const Mat4x4& v) {
        return out << "\rMatrix4x4{" << v.m00() << ", " << v.m01() << ", " << v.m02() << ", " << v.m03() << "} \n"\
                   << "         {" << v.m10() << ", " << v.m11() << ", " << v.m12() << ", " << v.m13() << "} \n"\
                   << "         {" << v.m20() << ", " << v.m21() << ", " << v.m22() << ", " << v.m23() << "} \n"\
                   << "         {" << v.m30() << ", " << v.m31() << ", " << v.m32() << ", " << v.m33() << "} \n";
    }
    
};

#endif //STEP_2_VECTORS_H

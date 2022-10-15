/*
 * Created by Brett Terpstra 6920201 on 14/10/22.
 * Copyright (c) Brett Terpstra 2022 All Rights Reserved
 */

#ifndef STEP_2_VECTORS_H
#define STEP_2_VECTORS_H

#include <cmath>
#include "util/std.h"

namespace Raytracing {

    // when running on the CPU it's fine to be a double
    // if your CPU is older (32bit) and has issues with doubles, consider changing it to a float
    // but if we move to the GPU it has to be a float.
    // since GPUs generally are far more optimized for floats
    typedef double PRECISION_TYPE;

    class vec4 {
    private:
        union xType {PRECISION_TYPE x; PRECISION_TYPE r; };
        union yType {PRECISION_TYPE y; PRECISION_TYPE g; };
        union zType {PRECISION_TYPE z; PRECISION_TYPE b; };
        union wType {PRECISION_TYPE w; PRECISION_TYPE a; };
        struct valueType {xType v1; yType v2; zType v3; wType v4;};
    public:
        // isn't much of a reason to do it this way
        // beyond I wanted an explicit immutable vector type of length 4
        // that could be used as both x,y,z + w? and rgba
        // it's unlikely that we'll need to use the w component
        // but it helps better line up with the GPU
        // and floating point units (especially on GPUs) tend to be aligned to 4*sizeof(float)
        const valueType value;

        vec4(): value{0,0,0,0} {}
        vec4(PRECISION_TYPE x, PRECISION_TYPE y, PRECISION_TYPE z): value{x,y,z,0} {}
        vec4(PRECISION_TYPE x, PRECISION_TYPE y, PRECISION_TYPE z, PRECISION_TYPE w): value{x,y,z,w} {}
        vec4(const vec4& vec): value{vec.x(), vec.y(), vec.z(), vec.w()} {}
        vec4 operator=(const vec4& other) { return {other.x(), other.y(), other.z(), other.w()}; }

        // I remember reading somewhere that if you can make it constant you should (Helps with -o flags?)
        // I'm still a little new to C++. TODO: compare compiler output
        // this is my second major project in it (the first being my java game engine i ported to c++)
        // since value is constant it's unlikely we actually need to
        const inline PRECISION_TYPE x() const {return value.v1.x;}
        const inline PRECISION_TYPE y() const {return value.v2.y;}
        const inline PRECISION_TYPE z() const {return value.v3.z;}
        const inline PRECISION_TYPE w() const {return value.v4.w;}

        const inline PRECISION_TYPE r() const {return value.v1.r;}
        const inline PRECISION_TYPE g() const {return value.v2.g;}
        const inline PRECISION_TYPE b() const {return value.v3.b;}
        const inline PRECISION_TYPE a() const {return value.v4.a;}

        // negation operator
        const vec4 operator-() const { return vec4(-x(), -y(), -z(), -w()); }

        const inline PRECISION_TYPE magnitude() const {
            return sqrt(length_squared());
        }

        const inline PRECISION_TYPE length_squared() const {
            return x() * x() + y() * y() + z() * z() + w() * w();
        }

        // returns the unit-vector.
        const inline vec4 normalize(){
            PRECISION_TYPE mag = magnitude();
            return vec4(x() / mag, y() / mag, z() / mag, w() / mag);
        }

        // add operator before the vec returns the magnitude
        PRECISION_TYPE operator+() const {
            return magnitude();
        }

        // preforms the dot product of left * right
        static inline const PRECISION_TYPE dot(const vec4& left, const vec4& right) {
            return left.x() * right.x()
                   + left.y() * right.y()
                   + left.z() * right.z()
                   + left.w() * right.w();
        }

        // preforms the cross product of left X right
        // since a general solution to the cross product doesn't exist in 4d
        // we are going to ignore the w.
        static inline const vec4 cross(const vec4& left, const vec4& right) {
            return vec4(left.y() * right.z() - left.z() * right.y(),
                        left.z() * right.x() - left.x() * right.z(),
                        left.x() * right.y() - left.y() * right.x());
        }

    };

// Utility Functions

    // useful for printing out the vector to stdout
    inline std::ostream& operator<<(std::ostream& out, const vec4& v) {
        return out << "vec4{" << v.x() << ", " << v.y() << ", " << v.z() << ", " << v.w() << "} ";
    }

    // adds the two vectors left and right
    inline const vec4 operator+(const vec4& left, const vec4& right) {
        return vec4(left.x() + right.x(), left.y() + right.y(), left.z() + right.z(), left.w() + right.w());
    }

    // subtracts the right vector from the left.
    inline const vec4 operator-(const vec4& left, const vec4& right) {
        return vec4(left.x() - right.x(), left.y() - right.y(), left.z() - right.z(), left.w() - right.w());
    }

    // multiples the left with the right
    inline const vec4 operator*(const vec4& left, const vec4& right) {
        return vec4(left.x() * right.x(), left.y() * right.y(), left.z() * right.z(), left.w() * right.w());
    }

    // multiplies the const c with each element in the vector v
    inline const vec4 operator*(const PRECISION_TYPE c, const vec4& v) {
        return vec4(c * v.x(), c * v.y(), c * v.z(), c * v.w());
    }

    // same as above but for right sided constants
    inline const vec4 operator*(const vec4& v, PRECISION_TYPE c) {
        return c * v;
    }

    // divides the vector by the constant c
    inline const vec4 operator/(const vec4& v, PRECISION_TYPE c) {
        return vec4(v.x() / c, v.y() / c, v.z() / c, v.w() / c);
    }

    // divides the constant by the magnitude of the vector
    inline const PRECISION_TYPE operator/(PRECISION_TYPE c, const vec4& v) {
        return c / +v;
    }

}

#endif //STEP_2_VECTORS_H

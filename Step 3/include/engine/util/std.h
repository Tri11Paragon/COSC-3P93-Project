/*
 * Created by Brett Terpstra 6920201 on 14/10/22.
 * Copyright (c) Brett Terpstra 2022 All Rights Reserved
 *
 * This file is used to include common standard library headers
 * There are some things {String, Maps} that I use a lot
 * Plus common defines that might be useful in the future.
 *
 */

#ifndef STEP_2_STD_H
#define STEP_2_STD_H

/**
 * includes
 */
#include <unordered_map>
#include <vector>
#include "logging.h"
#include <string>
#include <algorithm>
#include <cctype>
#include <locale>
#include <sstream>
#include <algorithm>
#include <limits>
#include <random>
#include <cstdlib>
#include <memory>

/**
 * defines
 */
#define RAYTRACING_VERSION_MAJOR 0
#define RAYTRACING_VERSION_MINOR 0
#define RAYTRACING_VERSION_PATCH 1
#define RAYTRACING_VERSION_STRING "0.0.1"

/**
 * Constants
 */
const double infinity = std::numeric_limits<double>::infinity();
const double ninfinity = -std::numeric_limits<double>::infinity();
// PI, to a large # of digits
const double PI = 3.1415926535897932385;
// very small number
const double EPSILON = 0.0000001;

/**
 * classes
 */
static inline double degreeeToRadian(double deg) {
    return deg * PI / 180.0;
}

namespace Raytracing {
    class AlignedAllocator {
        private:
        public:
            // not sure if this actually provides a performance benefit. Testing is inconclusive
            template<typename T>
            static T* allocateCacheAligned(int number = 1) {
                void* allocatedSpace = aligned_alloc(64, sizeof(T) * number);
                return new(allocatedSpace) T[number];
            }
    };
    
    class Random {
        private:
            std::random_device rd; // obtain a random number from hardware
            std::mt19937 gen;
            std::uniform_real_distribution<double> doubleDistr{0, 1};
        public:
            Random(): gen(std::mt19937(long(rd.entropy() * 691 * 691))) {}
            Random(double min, double max): gen(std::mt19937(long(rd.entropy() * 691 * 691))), doubleDistr{min, max} {}
            double getDouble() {
                return doubleDistr(gen);
            }
    };
    
    class String {
        public:
            static inline std::string toLowerCase(const std::string& s) {
                std::stringstream str;
                std::for_each(s.begin(), s.end(), [&str](unsigned char ch) {
                    str << (char) std::tolower(ch);
                });
                return str.str();
            }
            static inline std::string toUpperCase(const std::string& s) {
                std::stringstream str;
                std::for_each(s.begin(), s.end(), [&str](unsigned char ch) {
                    str << (char) std::toupper(ch);
                });
                return str.str();
            }
            // taken from https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
            // extended to return a vector
            static inline std::vector<std::string> split(std::string s, const std::string& delim) {
                size_t pos = 0;
                std::vector<std::string> tokens;
                while ((pos = s.find(delim)) != std::string::npos) {
                    auto token = s.substr(0, pos);
                    tokens.push_back(token);
                    s.erase(0, pos + delim.length());
                }
                tokens.push_back(s);
                return tokens;
            }
            // taken from https://stackoverflow.com/questions/216823/how-to-trim-an-stdstring
            // would've preferred to use boost lib but instructions said to avoid external libs
            // trim from start (in place)
            static inline std::string& ltrim(std::string& s) {
                s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
                    return !std::isspace(ch);
                }));
                return s;
            }
            
            // trim from end (in place)
            static inline std::string& rtrim(std::string& s) {
                s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
                    return !std::isspace(ch);
                }).base(), s.end());
                return s;
            }
            
            // trim from both ends (in place)
            static inline std::string& trim(std::string& s) {
                ltrim(s);
                rtrim(s);
                return s;
            }
            
            // trim from start (copying)
            static inline std::string ltrim_copy(std::string s) {
                ltrim(s);
                return s;
            }
            
            // trim from end (copying)
            static inline std::string rtrim_copy(std::string s) {
                rtrim(s);
                return s;
            }
            
            // trim from both ends (copying)
            static inline std::string trim_copy(std::string s) {
                trim(s);
                return s;
            }
    };
}

static Raytracing::Random rnd{};

static inline double getRandomDouble() {
    return rnd.getDouble();
}

static inline double clamp(double val, double min, double max) {
    if (val < min)
        return min;
    if (val > max)
        return max;
    return val;
}

#endif //STEP_2_STD_H

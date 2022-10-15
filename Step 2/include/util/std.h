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
#include "util/logging.h"
#include <string>
#include <algorithm>
#include <cctype>
#include <locale>
#include <sstream>
#include <algorithm>

/**
 * defines
 */
#define RAYTRACING_VERSION_MAJOR 0
#define RAYTRACING_VERSION_MINOR 0
#define RAYTRACING_VERSION_PATCH 1
#define RAYTRACING_VERSION_STRING "0.0.1"

/**
 * classes
 */
namespace Raytracing {
    class String {
        public:
            static inline std::string toLowerCase(const std::string& s){
                std::stringstream str;
                std::for_each(s.begin(), s.end(), [&str](unsigned char ch) {
                    str << std::tolower(ch);
                });
                return str.str();
            }
            static inline std::string toUpperCase(const std::string& s){
                std::stringstream str;
                std::for_each(s.begin(), s.end(), [&str](unsigned char ch) {
                    str << std::toupper(ch);
                });
                return str.str();
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

#endif //STEP_2_STD_H

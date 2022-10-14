/*
 * Created by Brett Terpstra 6920201 on 14/10/22.
 * Copyright (c) Brett Terpstra 2022 All Rights Reserved
 */

#ifndef STEP_2_PARSER_H
#define STEP_2_PARSER_H

#include "util/std.h"

namespace Raytracing {

    /**
     * The parser class is used to parse incoming values from the commandline.
     * It is not required that you use it for provided values
     * and in fact should work for values provided via stdin
     * however they must be provided in single line SSV char* form.
     */
    class Parser {
    private:
        std::unordered_map<std::string, std::string> raw_values;
        std::unordered_map<std::string, std::vector<std::string>> parsedValues;
    public:
        Parser();
        void addOption(std::string option, std::string description);
        bool hasOption(std::string option);
        std::string getOptionValue(std::string option);

        int parse(char** args, int argc);
    };

}

#endif //STEP_2_PARSER_H

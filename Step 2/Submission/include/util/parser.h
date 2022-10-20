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
        std::unordered_map<std::string, std::string> defaultValues;
        std::vector<std::string> descriptions;
    public:
        Parser();
        // Adds an option (or options) with default values.
        // default value defaults to an empty string and will not be added
        // useful if you want to add a description to the help menu.
        void addOption(const std::string& option, const std::string& description, const std::string& defaultValue = "");
        void addOption(const std::vector<std::string>& options, const std::string& description, const std::string& defaultValue = "");
        // returns true if the option provided is different from the default option.
        bool hasOptionChanged(const std::string& option);
        // checks if the option has been provided to the parser
        bool hasOption(const std::string& option);
        // check if any of the options exist, only use for checking options that lead to the same path
        // as this will return true at first option.
        bool hasOption(const std::vector<std::string>& options);
        // does not check to see if the option exists.
        std::string getOptionValue(const std::string& option);

        // parse the options from args
        int parse(char** args, int argc);
        void printDifferenceInInfo();
        void printAllInInfo();
    };

}

#endif //STEP_2_PARSER_H

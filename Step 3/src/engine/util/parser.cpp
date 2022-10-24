/*
 * Created by Brett Terpstra 6920201 on 14/10/22.
 * Copyright (c) Brett Terpstra 2022 All Rights Reserved
 */
#include "engine/util/parser.h"
#include <sstream>
namespace Raytracing {


    Parser::Parser() {
        // only to add the description (very annoying we have to have double {}!)
        addOption({{"--help"}, {"-h"}}, "Show this help menu!");
    }

    std::string Parser::getOptionValue(const std::string& option) {
        return raw_values.contains(option) ? raw_values.at(option) : defaultValues.at(option);
    }

    int Parser::parse(char **args, int argc) {
        // first arg is always the command path
        // which we don't care about.
        for (int i = 1; i < argc; i++) {
            std::string option = std::string(args[i]);
            std::string value = "true";
            // we only want to get the value if it was actually provided.
            if (i + 1 < argc) {
                value = std::string(args[i + 1]);
                // must make sure we don't skip over valuable parasble options!
                if (!value.starts_with("-")) {
                    // we must add to the value of i since we don't want to try and parse values.
                    i++;
                } else // if it does start with -, it's an option, we need to reset to true.
                    value = "true";
            }
            // if values are provided to us with -- or - we simply don't care about them!
            if (option.starts_with("-"))
                raw_values.insert(std::pair(option, value));
            else
                throw std::runtime_error("Unfortunately an invalid argument was provided! {" + option + "}");
        }
        if (hasOption({{"--help"}, {"-h"}})){
            std::cout << "Raytracer help information:" << std::endl;
            for (const auto& d : descriptions){
                std::cout << d;
            }
            return true;
        }
        return false;
    }

    void Parser::addOption(const std::string& option, const std::string& description, const std::string& defaultValue) {
        // we only want to add if the default value isn't empty,
        // since we might want to use the option as a single optional flag and not a value.
        if (!Raytracing::String::trim_copy(defaultValue).empty())
            defaultValues.insert(std::pair(option, defaultValue));
        descriptions.emplace_back(option + " :" + description);
    }

    bool Parser::hasOptionChanged(const std::string& option) {
        return raw_values.contains(option) && defaultValues.contains(option) && raw_values.at(option) != defaultValues.at(option);
    }

    void Parser::addOption(const std::vector<std::string>& options, const std::string &description, const std::string &defaultValue) {
        // add option like normal
        for (const auto& option : options){
            if (!Raytracing::String::trim_copy(defaultValue).empty())
                defaultValues.insert(std::pair(option, defaultValue));
        }
        // but improve the description for multi-option
        std::stringstream desStr;

        // add all the options to the description, seperated by ","
        for (int i = 0; i < options.size(); i++){
            desStr << options[i];
            if (i != options.size()-1)
                desStr << ", ";
        }
        desStr << " :";
        desStr << description;

        descriptions.emplace_back(desStr.str());
    }

    bool Parser::hasOption(const std::string &option) {
        return raw_values.contains(option);
    }

    bool Parser::hasOption(const std::vector<std::string> &options) {
        for (const auto& option : options)
            if (hasOption(option))
                return true;
        return false;
    }

    void Parser::printAllInInfo() {
        // print all with default values
        for (const auto& opt : defaultValues){
            ilog << opt.first << ": " << (raw_values.contains(opt.first) ? raw_values.at(opt.first) : opt.second) << " (default: " << opt.second << ")" << std::endl;
        }
        // then just print all the ones which don't have but where provided
        for (const auto& opt : raw_values){
            if (!defaultValues.contains(opt.first))
                ilog << "With: " << opt.first;
        }
    }

    void Parser::printDifferenceInInfo() {
        for (const auto& opt : raw_values){
            if (defaultValues.contains(opt.first))
                if (opt.second == defaultValues.at(opt.first))
                    ilog << opt.first << ": " << opt.second << " (default: " << defaultValues.at(opt.first) << ")" << std::endl;
        }
    }

}

/*
 * Created by Brett Terpstra 6920201 on 20/11/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */
#include <engine/util/loaders.h>
#include <fstream>
#include <ios>

namespace Raytracing {
    
    std::unordered_map<std::string, std::string> defines;
    
    std::vector<std::string> getLinesFromFile(const std::string& path){
        std::string shaderSource;
        std::ifstream shaderFile;
        RTAssert(shaderFile.good());
        // ensure ifstream objects can throw exceptions:
        shaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        try {
            // open file
            shaderFile.open(path);
            std::stringstream shaderStream;
            // read file's buffer contents into streams
            shaderStream << shaderFile.rdbuf();
            // close file handlers
            shaderFile.close();
            // convert stream into std::string
            shaderSource = shaderStream.str();
        } catch(std::ifstream::failure& e) {
            flog << "Unable to read file! " << path << "\n";
        }
    
        // split the shader into the lines, this way we can get out the #include statements.
        return String::split(shaderSource, "\n");
    }
    
    std::vector<std::string> includeDescent(const std::string& path){
        std::string pathOnly = path.substr(0, path.find_last_of('/'));
    
        auto mainLines = getLinesFromFile(path);
        std::unordered_map<int, std::vector<std::string>> includes;
    
        for (int i = 0; i < mainLines.size(); i++){
            auto& line = mainLines[i];
            // if the line is an include statement then we want to add lines recursively.
            if (line.starts_with("#include")){
                std::vector<std::string> statement1 = String::split(line, "<");
                std::vector<std::string> statement2 = String::split(line, "\"");
                String::trim(line);
                if ( !(line.ends_with(">") || line.ends_with("\"")) ) {
                    flog << "Shader file contains an invalid #include statement. (Missing terminator)\n";
                    throw std::runtime_error("");
                }
                try {
                    // filter out the > | " at the end of the include statement.
                    std::string file = statement1.empty() ? statement2[1].substr(0, statement2[1].size()-1) : statement1[1].substr(0, statement1[1].size()-1);
                
                    tlog << "Recusing into " << (pathOnly + "/" + file) << "\n";
    
                    includes.insert({i, includeDescent((pathOnly + "/" + file))});
                } catch (std::exception& e){
                    flog << "Shader file contains an invalid #include statement. (Missing < or \")\n";
                }
            }
        }
        
        std::vector<std::string> returnLines;
        
        // now combine all the loaded files while respecing the include's position in the file.
        for (int i = 0; i < mainLines.size(); i++){
            if (includes.contains(i)){
                auto includedFileLines = includes[i];
                
                for (const auto& line : includedFileLines)
                    returnLines.push_back(line);
            } else
                returnLines.push_back(mainLines[i]);
        }
        return returnLines;
    }
    
    std::string ShaderLoader::loadShaderFile(const std::string& path) {
        std::stringstream stringStream;
        
        auto lines = includeDescent(path);
        
        for (const auto& line : lines){
            // now process the defines, if they exist
            if (line.starts_with("#define")){
                auto defineParts = String::split(line, " ");
                // create a new define statement in the defines place but with the respective value.
                if (defines.contains(defineParts[1])){
                    stringStream << "#define ";
                    stringStream << defineParts[1] << " ";
                    stringStream << defines[defineParts[1]];
                }
            } else {
                stringStream << line;
                stringStream << "\n";
            }
        }
        
        tlog << stringStream.str();
        
        return stringStream.str();
    }
    void ShaderLoader::define(const std::string& key, const std::string& replacement) {
        defines.insert({key, replacement});
    }
}
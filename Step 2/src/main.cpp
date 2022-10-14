#include "util/std.h"
#include "util/parser.h"

int main(int argc, char** args) {
    Raytracing::Parser parser;

    // if the parser returns non-zero then it wants us to stop execution
    // likely due to a help function being called.
    if (parser.parse(args, argc))
        return 0;

    tlog << "Hello, World!" << std::endl;

    return 0;
}

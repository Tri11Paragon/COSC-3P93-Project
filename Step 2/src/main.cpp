#include "util/std.h"
#include "util/parser.h"
#include "image/image.h"

/**
 * Brett Terpstra 6920201
 * Hello TA / Marker! Welcome to the mess that is C++!
 * I'm still quite new to it (who isn't), and my laptop is setup
 * differently from my main computer. You'll notice comments generated
 * here are more detailed and very similar to Javadocs.
 * Half the comments are just reminders to myself for when I come back to do the parallel stuff anyways.
 * The formatting differences are also due to this, my computer is setup much nicer and closer to Java
 * while my laptop is just stock clion.
 */

int main(int argc, char** args) {
    // not a feature full parser but it'll work for what I need.
    Raytracing::Parser parser;

    parser.addOption("--single", "Enable Single Thread\n\tUse a single thread for ray tracing\n", "true");
    // not implemented yet
    parser.addOption("--multi", "Enable Multi-threading\n"
                                "\tUse multiple threads for ray tracing,\n"
                                "\tYou can set the max threads using -t or --threads\n");
    parser.addOption({{"-t"}, {"--threads"}}, "Max Usable Threads\n"
                                              "\tSet the max threads the ray tracer will attempt to use.\n"
                                              "\tDefaults to all cores of your cpu.\n", "0");
    // not implemented yet
    parser.addOption({{"--gui"}, {"-g"}}, "Enable GLFW GUI\n"
                                          "\tWill create a GUI using GLFW and display the image there.\n"
                                          "\tRequires the you compile with GLFW enabled. Will do nothing otherwise\n");
    // not implemented yet
    parser.addOption({{"--gpu"}, {"-c"}}, "Enables GPU Compute\n"
                                          "\tRequires the --gui/-g flag enabled,\n"
                                          "\tWill use OpenGL compute shaders to render the image\n");
    parser.addOption("--output", "Output Directory\n"
                                 "\tSet the output directory for the rendered image. Defaults to the local directory.\n", "./");
    parser.addOption("--format", "Output Format\n"
                                 "\tSets the output format to PPM, PNG, or JPEG. Currently only PPM is supported.", "PPM");

    // if the parser returns non-zero then it wants us to stop execution
    // likely due to a help function being called.
    if (parser.parse(args, argc))
        return 0;

    tlog << "Parsing complete! Starting raytracer with options:" << std::endl;
    // not perfect (contains duplicates) but good enough.
    parser.printAllInInfo();

    Raytracing::Image image(512, 512);

    for (int i = 0; i < image.getWidth(); i++){
        for (int j = 0; j < image.getHeight(); j++){
            image.setPixelColor(i, j, Raytracing::vec4(double(i) / double(image.getWidth()-1), double(j) / double(image.getHeight()-1), 0.25, 1.0));
        }
    }

    Raytracing::ImageOutput imageOutput(image);

    imageOutput.write("hello", "png");

    return 0;
}

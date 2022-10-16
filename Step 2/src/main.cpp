#include "util/std.h"
#include "util/parser.h"
#include "image/image.h"
#include <raytracing.h>

/**
 * Brett Terpstra 6920201
 *
 */

Raytracing::vec4 getRayColor(const Raytracing::Ray& ray){
    Raytracing::SphereObject obj(Raytracing::vec4(0,0,-1,0), 0.5);
    auto hit = obj.checkIfHit(ray, 0, 1000);
    if (hit.hit) {
        return 0.5*Raytracing::vec4(hit.normal.x()+1, hit.normal.y()+1, hit.normal.z()+1);
    }

    Raytracing::vec4 dir = ray.getDirection().normalize();
    auto t = 0.5f * (dir.y() + 1.0);
    return (1.0 - t) * Raytracing::vec4(1.0, 1.0, 1.0) + t * Raytracing::vec4(0.5, 0.7, 1.0);
}

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

    Raytracing::Image image(1366, 768);

    Raytracing::Camera camera(90, image);
    camera.lookAt(Raytracing::vec4(0,2,0), Raytracing::vec4(0, 0, -1), Raytracing::vec4(0, 1, 0));

    for (int i = 0; i < image.getWidth(); i++){
        for (int j = 0; j < image.getHeight(); j++){
            image.setPixelColor(i, j, getRayColor(camera.projectRay(i, j)));
        }
    }

    Raytracing::ImageOutput imageOutput(image);

    imageOutput.write("test", "png");

    return 0;
}

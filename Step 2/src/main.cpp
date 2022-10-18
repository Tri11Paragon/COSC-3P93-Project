#include "util/std.h"
#include "util/parser.h"
#include "image/image.h"
#include <raytracing.h>
#include <world.h>

/**
 * Brett Terpstra 6920201
 *
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

    Raytracing::Image image(445, 256);

    Raytracing::Camera camera(140, image);
    camera.setPosition({0, 0, 1});
    //camera.lookAt(Raytracing::vec4(0,1,0), Raytracing::vec4(0, 0, -1), Raytracing::vec4(0, 1, 0));

    Raytracing::World world;

    Raytracing::OBJLoader loader;
    Raytracing::ModelData testData = loader.loadModel("spider.obj");

    world.addMaterial("greenDiffuse", new Raytracing::DiffuseMaterial{Raytracing::vec4{0, 1.0, 0, 1}});
    world.addMaterial("redDiffuse", new Raytracing::DiffuseMaterial{Raytracing::vec4{1.0, 0, 0, 1}});
    world.addMaterial("blueDiffuse", new Raytracing::DiffuseMaterial{Raytracing::vec4{0, 0, 1.0, 1}});

    world.addMaterial("greenMetal", new Raytracing::MetalMaterial{Raytracing::vec4{0.4, 1.0, 0.4, 1}});
    world.addMaterial("redMetal", new Raytracing::BrushedMetalMaterial{Raytracing::vec4{1.0, 0.4, 0.4, 1}, 0.6f});
    world.addMaterial("blueMetal", new Raytracing::MetalMaterial{Raytracing::vec4{0.4, 0.4, 1.0, 1}});

    //world.add(new Raytracing::SphereObject(Raytracing::vec4(0,0,-1,0), 0.5, world.getMaterial("redDiffuse")));
    //world.add(new Raytracing::SphereObject(Raytracing::vec4(-1,0,-1,0), 0.5, world.getMaterial("blueMetal")));
    //world.add(new Raytracing::SphereObject(Raytracing::vec4(1,0,-1,0), 0.5, world.getMaterial("redMetal")));
    //world.add(new Raytracing::SphereObject(Raytracing::vec4(0,-100.5,-1,0), 100, world.getMaterial("greenDiffuse")));
    //world.add(new Raytracing::TriangleObject(Raytracing::vec4(0,0.1,-0.5f,0), {{-0.5, -0.5, 0.0}, {0.5, -0.5, 0.0}, {0.0,  0.5, 0}}, world.getMaterial("greenDiffuse")));
    world.add(new Raytracing::ModelObject({0, 0, -1}, testData, world.getMaterial("redDiffuse")));

    Raytracing::Raycaster raycaster {camera, image, world, parser};

    raycaster.run();

    Raytracing::ImageOutput imageOutput(image);

    imageOutput.write("test", "png");

    return 0;
}

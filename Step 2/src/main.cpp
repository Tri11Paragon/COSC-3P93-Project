#include "util/std.h"
#include "util/parser.h"
#include "image/image.h"
#include <raytracing.h>
#include <world.h>
#include <chrono>

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
                                 "\tSets the output format to BMP, PNG, or JPEG. \n", "PNG");
    parser.addOption("-w", "Image Width\n"
                           "\tSets the width of the output image.\n", "910");
    parser.addOption("-h", "Image Height\n"
                           "\tSets the height of the output image.\n", "512");
    parser.addOption("--fov", "Camera FOV\n"
                              "\tSets the FOV used to render the camera.\n", "90");
    parser.addOption("--resources", "Resources Directory\n"
                                    "\tSets the directory where the resources are stored.\n"
                                    "\tThis can be relative.Must have trailing '/' \n", "../resources/");
    // disabled because don't currently have a way to parse vectors. TODO
    //parser.addOption("--position", "Camera Position\n\tSets the position used to render the scene with the camera.\n", "{0, 0, 0}");

    // if the parser returns non-zero then it wants us to stop execution
    // likely due to a help function being called.
    if (parser.parse(args, argc))
        return 0;

    tlog << "Parsing complete! Starting raytracer with options:" << std::endl;
    // not perfect (contains duplicates) but good enough.
    parser.printAllInInfo();

    //Raytracing::Image image(445, 256);
    Raytracing::Image image(std::stoi(parser.getOptionValue("-w")), std::stoi(parser.getOptionValue("-h")));

    Raytracing::Camera camera(std::stoi(parser.getOptionValue("--fov")), image);
    //camera.setPosition({0, 0, 1});
    camera.lookAt({6,5,6}, {0, 0, 0}, {0, 1, 0});

    Raytracing::World world;

    Raytracing::OBJLoader loader;
    // assumes you are running it from a subdirectory, "build" or "cmake-build-release", etc.
    Raytracing::ModelData spider = loader.loadModel(parser.getOptionValue("--resources") + "spider.obj");
    Raytracing::ModelData house = loader.loadModel(parser.getOptionValue("--resources") + "house.obj");
    Raytracing::ModelData plane = loader.loadModel(parser.getOptionValue("--resources") + "plane.obj");

    world.addMaterial("greenDiffuse", new Raytracing::DiffuseMaterial{Raytracing::Vec4{0, 1.0, 0, 1}});
    world.addMaterial("redDiffuse", new Raytracing::DiffuseMaterial{Raytracing::Vec4{1.0, 0, 0, 1}});
    world.addMaterial("blueDiffuse", new Raytracing::DiffuseMaterial{Raytracing::Vec4{0, 0, 1.0, 1}});

    world.addMaterial("greenMetal", new Raytracing::MetalMaterial{Raytracing::Vec4{0.4, 1.0, 0.4, 1}});
    world.addMaterial("redMetal", new Raytracing::BrushedMetalMaterial{Raytracing::Vec4{1.0, 0.4, 0.4, 1}, 0.6f});
    world.addMaterial("blueMetal", new Raytracing::MetalMaterial{Raytracing::Vec4{0.4, 0.4, 1.0, 1}});

    //world.add(new Raytracing::SphereObject({0,0,-1,0{}, 0.5, world.getMaterial("redDiffuse")));
    //world.add(new Raytracing::SphereObject({-1,0,-1,0}, 0.5, world.getMaterial("blueMetal")));
    //world.add(new Raytracing::SphereObject({1,0,-1,0}, 0.5, world.getMaterial("redMetal")));
    world.add(new Raytracing::SphereObject({0,-100.5,-1,0}, 100, world.getMaterial("greenDiffuse")));
    //world.add(new Raytracing::TriangleObject({0,0.1,-0.5f,0}, {{-0.5, -0.5, 0.0}, {0.5, -0.5, 0.0}, {0.0,  0.5, 0}}, world.getMaterial("greenDiffuse")));
    world.add(new Raytracing::ModelObject({0, 1, 0}, spider, world.getMaterial("redDiffuse")));
    world.add(new Raytracing::ModelObject({-5, 0.5, 0}, plane, world.getMaterial("greenMetal")));
    world.add(new Raytracing::ModelObject({5, 1, 0}, house, world.getMaterial("redDiffuse")));
    world.add(new Raytracing::ModelObject({0, 0, -5}, house, world.getMaterial("blueDiffuse")));
    world.add(new Raytracing::ModelObject({0, 0, 5}, house, world.getMaterial("blueDiffuse")));

    Raytracing::Raycaster raycaster {camera, image, world, parser};

    ilog << "Running raycaster!\n";
    raycaster.run();

    Raytracing::ImageOutput imageOutput(image);
    
    auto t = std::time(nullptr);
    auto now = std::localtime(&t);
    std::stringstream timeString;
    timeString << (1900 + now->tm_year);
    timeString << "-";
    timeString << (1 + now->tm_mon);
    timeString << "-";
    timeString << now->tm_mday;
    timeString << " ";
    timeString << now->tm_hour;
    timeString << ":";
    timeString << now->tm_min;
    timeString << ":";
    timeString << now->tm_sec;
    ilog << "Writing Image!\n";
    imageOutput.write(parser.getOptionValue("--output") + timeString.str(), parser.getOptionValue("--format"));

    return 0;
}

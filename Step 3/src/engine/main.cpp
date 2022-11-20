#include "engine/util/std.h"
#include "engine/util/parser.h"
#include "engine/image/image.h"
#include "engine/raytracing.h"
#include "engine/world.h"
#include <chrono>
#include "engine/util/debug.h"
#include <config.h>
#include <csignal>
//#include <sys/time.h>
//#include <sys/resource.h>

#ifdef COMPILE_GUI
        
    #include <graphics/graphics.h>
    #include <graphics/gl/gl.h>
    #include <graphics/gl/shader.h>

#endif

#ifdef COMPILE_OPENCL

    #include <opencl/cl.h>

#endif


/**
 * Brett Terpstra 6920201
 *
 */

namespace Raytracing{
    extern Signals* RTSignal;
}

using namespace Raytracing;

int main(int argc, char** args) {
    // since this is linux only we can easily set our process priority to be high with a syscall
    // requires root. TODO: find way to doing this without root even if asking for user privilege escalation
    //setpriority(PRIO_PROCESS, 0, -20);
    // not a feature full parser but it'll work for what I need.
    Raytracing::Parser parser;
    
    parser.addOption("--single", "Enable Single Thread\n\tUse a single thread for ray tracing\n", "true");
    // not implemented yet
    parser.addOption("--multi", "Enable Multi-threading\n"
                                "\tUse multiple threads for ray tracing,\n"
                                "\tYou can set the max threads using -t or --threads\n");
    parser.addOption({{"-t"},
                      {"--threads"}}, "Max Usable Threads\n"
                                      "\tSet the max threads the ray tracer will attempt to use.\n"
                                      "\tDefaults to all cores of your cpu.\n", "0");
    // not implemented yet
    parser.addOption({{"--gui"},
                      {"-g"}}, "Enable GUI\n"
                               "\tWill create a GUI using X11 and display the image there.\n"
                               "\tRequires the you compile with the option -DCOMPILE_GUI=ON. Will do nothing otherwise\n");
    // not implemented yet
    parser.addOption({{"--gpu"},
                      {"-c"}}, "Enables GPU Compute\n"
                               "\tRequires the --gui/-g flag enabled,\n"
                               "\tWill use OpenGL compute shaders to render the image\n");
    parser.addOption("--output", "Output Directory\n"
                                 "\tSet the output directory for the rendered image. Defaults to the local directory.\n", "./");
    parser.addOption("--format", "Output Format\n"
                                 "\tSets the output format to BMP, PNG, or JPEG. \n", "PNG");
    parser.addOption("-w", "Image Width\n"
                           "\tSets the width of the output image.\n", "1440");
    parser.addOption("-h", "Image Height\n"
                           "\tSets the height of the output image.\n", "720");
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
    
    if (signal(SIGTERM, [] (int sig) -> void {
        ilog<<"Computations complete.\nHalting now...\n";
        RTSignal->haltExecution = true;
    })==SIG_ERR) { elog<<"Unable to change signal handler.\n";   return 1; }
    if (signal(SIGINT, [] (int sig) -> void {
        ilog<<"Computations complete.\nHalting now...\n";
        RTSignal->haltExecution = true;
    })==SIG_ERR) { elog<<"Unable to change signal handler.\n";   return 1; }
    
    tlog << "Parsing complete! Starting raytracer with options:" << std::endl;
    // not perfect (contains duplicates) but good enough.
    parser.printAllInInfo();
    
    #ifdef COMPILE_OPENCL
        OpenCL::init();
    #endif
    
    #ifdef COMPILE_GUI
        XWindow* window;
        if (parser.hasOption("--gui") || parser.hasOption("-g"))
            window = new XWindow(1440, 720);
        Shader worldShader("../resources/shaders/world.vs", "../resources/shaders/world.fs");
    #endif
    
    
    Raytracing::Image image(1440, 720);
    //Raytracing::Image image(std::stoi(parser.getOptionValue("-w")), std::stoi(parser.getOptionValue("-h")));
    
    Raytracing::Camera camera(std::stoi(parser.getOptionValue("--fov")), image);
    //camera.setPosition({0, 0, 1});
    camera.setPosition({6, 5, 6});
    camera.lookAt({0, 0, 0});
    
    
    WorldConfig worldConfig {worldShader};
    worldConfig.useBVH = true;
    
    Raytracing::World world {worldConfig};
    
    // assumes you are running it from a subdirectory, "build" or "cmake-build-release", etc.
    Raytracing::ModelData spider = Raytracing::OBJLoader::loadModel(parser.getOptionValue("--resources") + "spider.obj");
    Raytracing::ModelData house = Raytracing::OBJLoader::loadModel(parser.getOptionValue("--resources") + "house.obj");
    Raytracing::ModelData plane = Raytracing::OBJLoader::loadModel(parser.getOptionValue("--resources") + "plane.obj");
    
    world.add("greenDiffuse", new Raytracing::DiffuseMaterial{Raytracing::Vec4{0, 1.0, 0, 1}});
    world.add("redDiffuse", new Raytracing::DiffuseMaterial{Raytracing::Vec4{1.0, 0, 0, 1}});
    world.add("blueDiffuse", new Raytracing::DiffuseMaterial{Raytracing::Vec4{0, 0, 1.0, 1}});
    
    world.add("greenMetal", new Raytracing::MetalMaterial{Raytracing::Vec4{0.4, 1.0, 0.4, 1}});
    world.add("redMetal", new Raytracing::BrushedMetalMaterial{Raytracing::Vec4{1.0, 0.4, 0.4, 1}, 0.6f});
    world.add("blueMetal", new Raytracing::MetalMaterial{Raytracing::Vec4{0.4, 0.4, 1.0, 1}});
    
    world.add(new Raytracing::SphereObject({0, -100.5, -1, 0}, 100, world.getMaterial("greenDiffuse")));
    
    world.add(new Raytracing::ModelObject({0, 1, 0}, spider, world.getMaterial("redDiffuse")));
    world.add(new Raytracing::ModelObject({-5, 0.5, 0}, plane, world.getMaterial("greenMetal")));
    world.add(new Raytracing::ModelObject({5, 1, 0}, house, world.getMaterial("redDiffuse")));
    world.add(new Raytracing::ModelObject({0, 0, -5}, house, world.getMaterial("blueDiffuse")));
    world.add(new Raytracing::ModelObject({0, 0, 5}, house, world.getMaterial("blueDiffuse")));
    
    if (parser.hasOption("--gui") || parser.hasOption("-g")) {
        #ifdef COMPILE_GUI
            Raytracing::Raycaster raycaster {camera, image, world, parser};
            Texture mainImage(&image);
            Shader shader("../resources/shaders/basic.vs", "../resources/shaders/basic.fs");
            Raytracing::DisplayRenderer renderer {*window, mainImage, world, shader, worldShader, raycaster, parser, camera};
            while (!window->shouldWindowClose()) {
                window->beginUpdate();
                renderer.draw();
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                world.drawBVH(worldShader);
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                window->endUpdate();
            }
            RTSignal->haltExecution= true;
            raycaster.join();
        #else
            flog << "Program not compiled with GUI support! Unable to continue!\n";
        #endif
    } else {
        Raytracing::Raycaster raycaster {camera, image, world, parser};
        // run the raycaster the standard way
        ilog << "Running raycaster!\n";
        // we don't actually have to check for --single since it's implied to be default true.
        raycaster.run(parser.hasOption("--multi"), std::max(std::stoi(parser.getOptionValue("-t")), std::stoi(parser.getOptionValue("--threads"))));
        raycaster.join();
    }
    
    profiler::print("Raytracer Results");
    
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
    
    delete(RTSignal);
    #ifdef COMPILE_GUI
        deleteQuad();
    #endif
    
    return 0;
}

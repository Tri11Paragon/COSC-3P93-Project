#include "engine/util/std.h"
#include "engine/util/parser.h"
#include "engine/image/image.h"
#include "engine/raytracing.h"
#include "engine/world.h"
#include <chrono>
#include "engine/util/debug.h"
#include "opencl/open_ray_tracing.h"
#include <config.h>
#include <csignal>

#ifdef USE_MPI

#include <engine/mpi.h>

#endif
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

namespace Raytracing {
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
    parser.addOption(
            "--multi", "Enable Multi-threading\n"
                       "\tUse multiple threads for ray tracing,\n"
                       "\tYou can set the max threads using -t or --threads\n"
    );
    parser.addOption(
            {{"-t"},
             {"--threads"}}, "Max Usable Threads\n"
                             "\tSet the max threads the ray tracer will attempt to use.\n"
                             "\tDefaults to all cores of your cpu.\n", "0"
    );
    // not implemented yet
    parser.addOption(
            {{"--gui"},
             {"-g"}}, "Enable GUI\n"
                      "\tWill create a GUI using X11 and display the image there.\n"
                      "\tRequires the you compile with the option -DCOMPILE_GUI=ON. Will do nothing otherwise\n"
    );
    // not implemented yet
    parser.addOption(
            {{"--gpu"},
             {"-c"}}, "Enables GPU Compute\n"
                      "\tWill use OpenCL compute to render the image\n"
    );
    parser.addOption(
            "--output", "Output Directory\n"
                        "\tSet the output directory for the rendered image. Defaults to the local directory.\n", "./"
    );
    parser.addOption(
            "--format", "Output Format\n"
                        "\tSets the output format to BMP, PNG, or JPEG. \n", "PNG"
    );
    parser.addOption(
            "-w", "Image Width\n"
                  "\tSets the width of the output image.\n", "1440"
    );
    parser.addOption(
            "-h", "Image Height\n"
                  "\tSets the height of the output image.\n", "720"
    );
    parser.addOption(
            "--fov", "Camera FOV\n"
                     "\tSets the FOV used to render the camera.\n", "90"
    );
    parser.addOption(
            "--resources", "Resources Directory\n"
                           "\tSets the directory where the resources are stored.\n"
                           "\tThis can be relative.Must have trailing '/' \n", "../resources/"
    );
    parser.addOption(
            "--mpi", "Use OpenMPI\n"
                     "\tTells the raycaster to use OpenMPI to run the raycaster algorithm\n"
    );
    parser.addOption(
            "--openmp", "Use OpenMP\n"
                        "\tTells the raycaster to use OpenMP to run the raycaster algorithm\n"
    );
    
    // disabled because don't currently have a way to parse vectors. TODO
    //parser.addOption("--position", "Camera Position\n\tSets the position used to render the scene with the camera.\n", "{0, 0, 0}");
    
    // if the parser returns non-zero then it wants us to stop execution
    // likely due to a help function being called.
    if (parser.parse(args, argc))
        return 0;
    
    if (signal(
            SIGTERM, [](int sig) -> void {
                ilog << "Computations complete.\nHalting now...\n";
                RTSignal->haltExecution = true;
            }
    ) == SIG_ERR) {
        elog << "Unable to change signal handler.\n";
        return 1;
    }
    if (signal(
            SIGINT, [](int sig) -> void {
                ilog << "Computations complete.\nHalting now...\n";
                RTSignal->haltExecution = true;
            }
    ) == SIG_ERR) {
        elog << "Unable to change signal handler.\n";
        return 1;
    }
    
    tlog << "Parsing complete! Starting raytracer with options:" << std::endl;
    // not perfect (contains duplicates) but good enough.
    parser.printAllInInfo();

#ifdef USE_MPI
    Raytracing::MPI::init(argc, args);
#endif

#ifdef COMPILE_GUI
    XWindow* window;
    if (parser.hasOption("--gui") || parser.hasOption("-g"))
        window = new XWindow(1440, 720);
    Shader worldShader("../resources/shaders/world.vs", "../resources/shaders/world.fs");
#endif

#ifdef COMPILE_OPENCL
    OpenCL::init();

#endif
    
    Raytracing::Image image(1440, 720);
    //Raytracing::Image image(std::stoi(parser.getOptionValue("-w")), std::stoi(parser.getOptionValue("-h")));
    
    Raytracing::Camera camera(std::stoi(parser.getOptionValue("--fov")), image);
    //camera.setPosition({0, 0, 1});
    camera.setPosition({6, 5, 6});
    camera.lookAt({0, 0, 0});


#ifdef COMPILE_GUI
    WorldConfig worldConfig{worldShader};
#else
    WorldConfig worldConfig;
#endif
    worldConfig.useBVH = true;
    
    Raytracing::World world{worldConfig};
    
    // assumes you are running it from a subdirectory, "build" or "cmake-build-release", etc.
    Raytracing::ModelData spider = Raytracing::OBJLoader::loadModel(parser.getOptionValue("--resources") + "spider.obj");
    Raytracing::ModelData house = Raytracing::OBJLoader::loadModel(parser.getOptionValue("--resources") + "house.obj");
    Raytracing::ModelData plane = Raytracing::OBJLoader::loadModel(parser.getOptionValue("--resources") + "plane.obj");
    Raytracing::ModelData debugCube = Raytracing::OBJLoader::loadModel(parser.getOptionValue("--resources") + "skybox.obj");
    
    world.add("greenDiffuse", new Raytracing::DiffuseMaterial{Raytracing::Vec4{0, 1.0, 0, 1}});
    world.add("redDiffuse", new Raytracing::DiffuseMaterial{Raytracing::Vec4{1.0, 0, 0, 1}});
    world.add("blueDiffuse", new Raytracing::DiffuseMaterial{Raytracing::Vec4{0, 0, 1.0, 1}});
    world.add("light", new Raytracing::LightMaterial{Raytracing::Vec4{10.0, 10.0, 10.0}});
    
    world.add("greenMetal", new Raytracing::MetalMaterial{Raytracing::Vec4{0.4, 1.0, 0.4, 1}});
    world.add("redMetal", new Raytracing::BrushedMetalMaterial{Raytracing::Vec4{1.0, 0.4, 0.4, 1}, 0.6f});
    world.add("blueMetal", new Raytracing::MetalMaterial{Raytracing::Vec4{0.4, 0.4, 1.0, 1}});
    world.add("magic", new Raytracing::TexturedMaterial{parser.getOptionValue("--resources") + "760213.png"});
    world.add("thinkers", new Raytracing::TexturedMaterial{parser.getOptionValue("--resources") + "1616466348379.png"});
    world.add("sponge", new Raytracing::TexturedMaterial{parser.getOptionValue("--resources") + "029a_-_Survival_of_the_Idiots_349.jpg"});
    world.add("cat", new Raytracing::TexturedMaterial{parser.getOptionValue("--resources") + "livingmylifeinstereodoesntseemthatbad.PNG"});
    
    world.add(new Raytracing::SphereObject({0, -100.5, -1, 0}, 100, world.getMaterial("greenDiffuse")));
    
    world.add(new Raytracing::ModelObject({0, 1, 0}, spider, world.getMaterial("redDiffuse")));
    world.add(new Raytracing::ModelObject({-5, 0.5, 0}, plane, world.getMaterial("greenMetal")));
    world.add(new Raytracing::ModelObject({5, 1, 0}, house, world.getMaterial("redDiffuse")));
    world.add(new Raytracing::ModelObject({0, 0, -5}, house, world.getMaterial("blueDiffuse")));
    world.add(new Raytracing::ModelObject({0, 0, 5}, house, world.getMaterial("blueDiffuse")));
    //world.add(new Raytracing::ModelObject({0, 0, 0}, debugCube, world.getMaterial("cat")));
    
    if (parser.hasOption("--gui") || parser.hasOption("-g")) {
#ifdef COMPILE_GUI
        Raytracing::RayCaster rayCaster{camera, image, world, parser};
        Texture mainImage(&image);
        
        
        OpenClRaytracer openClRaytracer{parser.getOptionValue("--resources") + "opencl/raytracer.cl", image, world};
        openClRaytracer.run();
        
        Shader shader("../resources/shaders/basic.vs", "../resources/shaders/basic.fs");
        Raytracing::DisplayRenderer renderer{*window, mainImage, world, shader, worldShader, rayCaster, parser, camera};
        while (!window->shouldWindowClose()) {
            window->beginUpdate();
            renderer.draw();
            
            
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            world.drawBVH(worldShader);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            window->endUpdate();
        }
        RTSignal->haltExecution = true;
        rayCaster.join();
#else
        flog << "Program not compiled with GUI support! Unable to open GUI\n";
#endif
    } else {
        Raytracing::RayCaster rayCaster{camera, image, world, parser};
        ilog << "Running RayCaster (NO_GUI)!\n";
        // we don't actually have to check for --single since it's implied to be default true.
        int threads = std::stoi(parser.getOptionValue("--threads"));
        if (parser.hasOption("--mpi")) {
            // We need to make sure that if the user requests that MPI be run while not having MPI compiled, they get a helpful error warning.
#ifdef USE_MPI
            rayCaster.runMPI(Raytracing::MPI::getCurrentImageRegionAssociation(rayCaster));
#else
            flog << "Unable to run with MPI, CMake not set to compile MPI!\n";
            return 33;
#endif
        } else if (parser.hasOption("--openmp")) {
            rayCaster.runOpenMP(threads);
        } else {
            rayCaster.runSTDThread(threads);
        }
        rayCaster.join();
    }
    
    profiler::print("Raytracer Results");

#ifdef USE_MPI
    // Wait for all processes to finish before trying to send data
    MPI_Barrier(MPI_COMM_WORLD);
    // don't send data to ourselves
    if (currentProcessID != 0) {
        auto imageArray = image.toArray();
        MPI_Send(imageArray.data(), (int)imageArray.size(), MPI_DOUBLE, 0, 1, MPI_COMM_WORLD);
    } else {
        auto doubleSize = (image.getWidth() + 1) * (image.getHeight() + 1) * 4;
        auto* buffer = new double[doubleSize];
        for (int i = 1; i < numberOfProcesses; i++){
            // get the data from all sending processes
            MPI_Recv(buffer, doubleSize, MPI_DOUBLE, i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            // copy that memory into the image
            image.fromArray(buffer, doubleSize, i);
        }
#endif
    // write the image to the file
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
#ifdef USE_MPI
    }
    // wait for all processes to finish sending and receiving before we exit all of them.
    MPI_Barrier(MPI_COMM_WORLD);
#endif
    
    delete (RTSignal);
#ifdef COMPILE_GUI
    deleteQuad();
#endif
#ifdef USE_MPI
    MPI_Finalize();
#endif
    
    tlog << "Goodbye!\n";
    return 0;
}

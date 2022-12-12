/*
 * Created by Brett Terpstra 6920201 on 16/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */
#include "engine/raytracing.h"
#include <queue>
#include <functional>
#include <utility>
#include <engine/util/debug.h>
#include <config.h>

#ifdef USE_MPI
    
    #include <mpi/mpi.h>
    #include <engine/mpi.h>

#else
    #ifdef USE_OPENMP
        #include <omp.h>
    #endif
#endif

namespace Raytracing {
    
    extern Signals* RTSignal;
    
    Ray Camera::projectRay(PRECISION_TYPE x, PRECISION_TYPE y) {
        // transform the x and y to points from image coords to be inside the camera's viewport.
        double transformedX = (x / (image.getWidth() - 1));
        double transformedY = (y / (image.getHeight() - 1));
        // then generate a ray which extends out from the camera position in the direction with respects to its position on the image
        return {position, imageOrigin + transformedX * horizontalAxis + transformedY * verticalAxis - position};
    }
    
    void Camera::lookAt(const Vec4& lookAtPos) {
        // standard camera lookAt function
        auto w = (position - lookAtPos).normalize();
        auto u = (Vec4::cross(up, w)).normalize();
        auto v = Vec4::cross(w, u);
        
        horizontalAxis = viewportWidth * u;
        verticalAxis = viewportHeight * v;
        imageOrigin = position - horizontalAxis / 2 - verticalAxis / 2 - w;
    }
    
    Mat4x4 Camera::view(PRECISION_TYPE yaw, PRECISION_TYPE pitch) {
        Mat4x4 view;
        
        pitch = degreeeToRadian(pitch);
        yaw = degreeeToRadian(yaw);
        
        PRECISION_TYPE cosPitch = std::cos(pitch);
        PRECISION_TYPE cosYaw = std::cos(yaw);
        PRECISION_TYPE sinPitch = std::sin(pitch);
        PRECISION_TYPE sinYaw = std::sin(yaw);
        
        auto x = Vec4{cosYaw, 0, -sinYaw}; // forward
        auto y = Vec4{sinYaw * sinPitch, cosPitch, cosYaw * sinPitch}; // right
        auto z = Vec4{sinYaw * cosPitch, -sinPitch, cosPitch * cosYaw}; // up
        
        // we can actually take those x, y, z vectors and use them to compute the raytracer camera settings
        viewportHeight = 2 * tanFovHalf;
        viewportWidth = aspectRatio * viewportHeight;
        // exactly the same as the look at function.
        horizontalAxis = viewportWidth * x;
        verticalAxis = viewportHeight * y;
        imageOrigin = position - horizontalAxis / 2 - verticalAxis / 2 - z;
        
        view.m00(float(x.x()));
        view.m01(float(x.y()));
        view.m02(float(x.z()));
        view.m03(float(x.w()));
        
        view.m10(float(y.x()));
        view.m11(float(y.y()));
        view.m12(float(y.z()));
        view.m13(float(y.w()));
        
        view.m20(float(z.x()));
        view.m21(float(z.y()));
        view.m22(float(z.z()));
        view.m23(float(z.w()));
        
        // view matrix are inverted, dot product to simulate translate matrix multiplication
        view.m03(-float(Vec4::dot(x, position)));
        view.m13(-float(Vec4::dot(y, position)));
        view.m23(-float(Vec4::dot(z, position)));
        view.m33(1);
        
        return view;
    }
    
    struct RayData {
        Ray ray;
        int depth;
        Vec4 color;
    };
    
    Vec4 RayCaster::raycasti(const Ray& ray, int depth) {
        return {};
    }
    
    Vec4 RayCaster::raycast(const Ray& ray) {
        Ray localRay = ray;
        Vec4 color{1.0, 1.0, 1.0};
        for (int CURRENT_BOUNCE = 0; CURRENT_BOUNCE < maxBounceDepth; CURRENT_BOUNCE++) {
            if (RTSignal->haltExecution || RTSignal->haltRaytracing)
                return color;
            while (RTSignal->pauseRaytracing) // sleep for 1/60th of a second, or about 1 frame.
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
            
            auto hit = world.checkIfHit(localRay, 0.001, infinity);
            if (hit.first.hit) {
                auto object = hit.second;
                auto scatterResults = object->getMaterial()->scatter(localRay, hit.first);
                //auto emission = object->getMaterial()->emission(hit.first.u, hit.first.v, hit.first.hitPoint);
                // if the material scatters the ray, ie casts a new one,
                if (scatterResults.scattered) { // attenuate the recursive raycast by the material's color
                    color = color * scatterResults.attenuationColor;
                    localRay = scatterResults.newRay;
                } else {
                    // if we don't scatter, we don't need to keep looping
                    // but we should return whatever the material's emission is
                    // which for all that aren't lights (currently) is the old black color.
                    //color = color + emission;
                    color = {};
                    break;
                }
            } else {
                // since we didn't hit, we hit the sky.
                color = color * Vec4{0.5, 0.7, 1.0};
                //color = Vec4{};
                // if we don't hit we cannot keep looping.
                break;
            }
        }
        
        return color;
    }
    
    void RayCaster::runRaycastingAlgorithm(RaycasterImageBounds imageBounds, int loopX, int loopY) {
        try {
            int x = imageBounds.x + loopX;
            int y = imageBounds.y + loopY;
            Raytracing::Vec4 color;
            for (int s = 0; s < raysPerPixel; s++) {
                // simulate anti aliasing by generating rays with very slight random directions
                color = color + raycast(camera.projectRay(x + rnd.getDouble(), y + rnd.getDouble()));
            }
            PRECISION_TYPE sf = 1.0 / raysPerPixel;
            // apply pixel color with gamma correction
            image.setPixelColor(x, y, {std::sqrt(sf * color.r()), std::sqrt(sf * color.g()), std::sqrt(sf * color.b())});
            if (RTSignal->haltExecution || RTSignal->haltRaytracing)
                return;
            while (RTSignal->pauseRaytracing) // sleep for 1/60th of a second, or about 1 frame.
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
        } catch (std::exception& error) {
            flog << "Possibly fatal error in the multithreaded raytracer!\n";
            flog << error.what() << "\n";
        }
    }
    
    
    void RayCaster::runSTDThread(int threads) {
        setupQueue(partitionScreen(threads));
        updateThreadValue(threads);
        ilog << "Running std::thread\n";
        for (int i = 0; i < threads; i++) {
            executors.push_back(
                    std::make_unique<std::thread>(
                            [this, i, threads]() -> void {
                                // run through all the quadrants
                                std::stringstream str;
                                str << "Threading of #";
                                str << (i + 1);
                                profiler::start("Raytracer Results", str.str());
                                while (unprocessedQuads != nullptr) {
                                    RaycasterImageBounds imageBoundingData{};
                                    // get the function for the quadrant
                                    queueSync.lock();
                                    if (unprocessedQuads->empty()) {
                                        queueSync.unlock();
                                        break;
                                    }
                                    imageBoundingData = unprocessedQuads->front();
                                    unprocessedQuads->pop();
                                    queueSync.unlock();
                                    // the run it
                                    for (int kx = 0; kx <= imageBoundingData.width; kx++) {
                                        for (int ky = 0; ky < imageBoundingData.height; ky++) {
                                            runRaycastingAlgorithm(imageBoundingData, kx, ky);
                                        }
                                    }
                                }
                                finishedThreads++;
                                profiler::end("Raytracer Results", str.str());
                            }
                    ));
        }
    }
    
    void RayCaster::runOpenMP(int threads) {
        setupQueue(partitionScreen(threads));
        updateThreadValue(threads);
#ifdef USE_OPENMP
        ilog << "Running OpenMP\n";
#pragma omp parallel num_threads(threads+1) default(none) shared(threads)
        {
            int threadID = omp_get_thread_num();
            // an attempt at making the omp command non-blocking.
            if (threadID != 0) {
                std::stringstream str;
                str << "Threading of #";
                str << (threadID);
                profiler::start("Raytracer Results", str.str());
                int j = 0;
                // run through all the quadrants
                bool running = true;
                while (running) {
                    RaycasterImageBounds imageBoundingData{};
#pragma omp critical
                    {
                        if (unprocessedQuads->empty())
                            running = false;
                        if (running) {
                            imageBoundingData = unprocessedQuads->front();
                            unprocessedQuads->pop();
                        }
                    }
                    if (running) {
                        for (int kx = 0; kx <= imageBoundingData.width; kx++) {
                            for (int ky = 0; ky < imageBoundingData.height; ky++) {
                                runRaycastingAlgorithm(imageBoundingData, kx, ky);
                            }
                        }
                    }
                }
#pragma omp critical
                finishedThreads++;
                profiler::end("Raytracer Results", str.str());
            }
        }
        tlog << "OpenMP finished!\n";
#else
        flog << "Not compiled with OpenMP! Unable to run raytracing.\n";
        system_threads;
#endif
    }
    
    void RayCaster::runMPI(std::queue<RaycasterImageBounds> bounds) {
#ifdef USE_MPI
        ilog << "Running MPI\n";
        dlog << "We have " << bounds.size() << " bounds currently pending!\n";
        while (!bounds.empty()) {
            auto region = bounds.front();
            for (int kx = 0; kx <= region.width; kx++) {
                for (int ky = 0; ky < region.height; ky++) {
                    runRaycastingAlgorithm(region, kx, ky);
                }
            }
            bounds.pop();
        }
        dlog << "Finished running MPI on " << currentProcessID << "\n";
#else
        flog << "Not compiled with MPI!\n";
#endif
    }
    
    std::vector<RaycasterImageBounds> RayCaster::partitionScreen(int threads) {
        // if we are running single threaded, disable everything special
        // the reason we run single threaded in a seperate thread is because the GUI requires its own set of updating commands
        // which cannot be blocked by the raytracer, otherwise it would become unresponsive.
        int divs = 1;
        if (threads < 0 || threads == 1) {
            threads = 1;
            divs = 1;
        } else {
            if (threads == 0)
                threads = (int) system_threads;
            // calculate the max divisions we can have per side, then expand by a factor of 4.
            // the reason to do this is that some of them will finish far quciker than others. The now free threads can keep working.
            // to do it without a queue like this leads to most threads finishing and a single thread being the critical path which isn't optimally efficient.
            divs = int(std::log(threads) / std::log(2)) * 4;
        }
        
        ilog << "Generating multithreaded raytracer with " << threads << " threads and " << divs << " divisions! \n";
        
        std::vector<RaycasterImageBounds> bounds;
        
        // we need to subdivide the image for the threads, since this is really quick it's fine to due sequentially
        for (int dx = 0; dx < divs; dx++) {
            for (int dy = 0; dy < divs; dy++) {
                bounds.push_back(
                        {
                                image.getWidth() / divs,
                                image.getHeight() / divs,
                                (image.getWidth() / divs) * dx,
                                (image.getHeight() / divs) * dy
                        }
                );
            }
        }
        return bounds;
    }
    
    void RayCaster::setupQueue(const std::vector<RaycasterImageBounds>& bounds) {
        delete (unprocessedQuads);
        unprocessedQuads = new std::queue<RaycasterImageBounds>();
        for (auto& b : bounds)
            unprocessedQuads->push(b);
    }
    
}

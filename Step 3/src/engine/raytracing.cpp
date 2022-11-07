/*
 * Created by Brett Terpstra 6920201 on 16/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */
#include "engine/raytracing.h"
#include <queue>
#include <functional>
#include <utility>
#include <engine/util/debug.h>

extern bool* haltExecution;
extern bool* pauseRaytracing;
extern bool* haltRaytracing;

namespace Raytracing {
    
    Ray Camera::projectRay(PRECISION_TYPE x, PRECISION_TYPE y) {
        // transform the x and y to points from image coords to be inside the camera's viewport.
        double transformedX = (x / (image.getWidth() - 1));
        auto transformedY = (y / (image.getHeight() - 1));
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
    
    void Camera::setRotation(const PRECISION_TYPE yaw, const PRECISION_TYPE pitch, const PRECISION_TYPE roll) {
        // TODO:
    }
    
    struct RayData {
        Ray ray;
        int depth;
        Vec4 color;
    };
    
    Vec4 Raycaster::raycast(const Ray& ray) {
        Ray localRay = ray;
        Vec4 color {1.0, 1.0, 1.0};
        for (int CURRENT_BOUNCE = 0; CURRENT_BOUNCE < maxBounceDepth; CURRENT_BOUNCE++){
            if (*haltExecution || *haltRaytracing)
                return color;
            while (*pauseRaytracing) // sleep for 1/60th of a second, or about 1 frame.
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
            auto hit = world.checkIfHit(localRay, 0.001, infinity);
            if (hit.first.hit) {
                auto object = hit.second;
                auto scatterResults = object->getMaterial()->scatter(localRay, hit.first);
                // if the material scatters the ray, ie casts a new one,
                if (scatterResults.scattered) { // attenuate the recursive raycast by the material's color
                    color = color * scatterResults.attenuationColor;
                    localRay = scatterResults.newRay;
                } else {
                    // if we don't scatter, we don't need to keep looping
                    color = {0.0, 0.0, 0.0};
                    break;
                }
            } else {
                // since we didn't hit, we hit the sky.
                color = color * Vec4{0.5, 0.7, 1.0};
                // if we don't hit we cannot keep looping.
                break;
            }
        }
        
        return color;
        // old recursive version:
        /*if (depth > maxBounceDepth)
            return {0, 0, 0};
        
        auto hit = world.checkIfHit(ray, 0.001, infinity);
        
        if (hit.first.hit) {
            auto object = hit.second;
            auto scatterResults = object->getMaterial()->scatter(ray, hit.first);
            // if the material scatters the ray, ie casts a new one,
            if (scatterResults.scattered) // attenuate the recursive raycast by the material's color
                return scatterResults.attenuationColor * raycast(scatterResults.newRay, depth + 1);
            //tlog << "Not scattered? " << object->getMaterial() << "\n";
            return {0, 0, 0};
        }
        
        // skybox color
        return {0.5, 0.7, 1.0};*/
    }
    
    void Raycaster::runSingle() {
        executors.push_back(std::make_unique<std::thread>([this]() -> void {
            profiler::start("Raytracer Results", "Single Thread");
            for (int i = 0; i < image.getWidth(); i++) {
                for (int j = 0; j < image.getHeight(); j++) {
                    Raytracing::Vec4 color;
                    // TODO: profile for speed;
                    for (int s = 0; s < raysPerPixel; s++) {
                        // simulate anti aliasing by generating rays with very slight random directions
                        color = color + raycast(camera.projectRay(i + rnd.getDouble(), j + rnd.getDouble()));
                    }
                    PRECISION_TYPE sf = 1.0 / raysPerPixel;
                    // apply pixel color with gamma correction
                    image.setPixelColor(i, j, {std::sqrt(sf * color.r()), std::sqrt(sf * color.g()), std::sqrt(sf * color.b())});
                    if (*haltExecution || *haltRaytracing)
                        return;
                    while (*pauseRaytracing) // sleep for 1/60th of a second, or about 1 frame.
                        std::this_thread::sleep_for(std::chrono::milliseconds(16));
                }
            }
            profiler::end("Raytracer Results", "Single Thread");
            finishedThreads++;
        }));
    }
    
    void Raycaster::runMulti(unsigned int t) {
        // calculate the max divisions we can have per side
        // say we have 16 threads, making divs 4
        // 4 divs per axis, two axis, 16 total quadrants
        // matching the 16 threads.
        if (t == 0)
            t = system_threads;
        ilog << "Starting multithreaded raytracer with " << t << " threads!\n";
        int divs = int(std::log(t) / std::log(2));
        // now double the divs, splitting each quadrant into 4 sub-quadrants which we can queue
        // the reason to do this is that some of them will finish before others, and the now free threads can keep working
        // do it without a queue like this leads to a single thread critical path and isn't optimally efficient.
        divs *= 4; // 2 because two axis getting split makes 4 sub-quadrants, but I tested 4, and it was faster by two seconds, so I'm keeping 4.
        
        delete(unprocessedQuads);
        unprocessedQuads = new std::queue<std::vector<int>>();
        
        for (int dx = 0; dx < divs; dx++) {
            for (int dy = 0; dy < divs; dy++) {
                // sending functions wasn't working. (fixed, however it feels janky sending lambda functions w/ captures)
                unprocessedQuads->push({
                                               image.getWidth() / divs,
                                               image.getHeight() / divs,
                                               (image.getWidth() / divs) * dx,
                                               (image.getHeight() / divs) * dy
                                       });
            }
        }
        
        for (int i = 0; i < t; i++) {
            executors.push_back(std::make_unique<std::thread>([this, i, divs, t]() -> void {
                // run through all the quadrants
                std::stringstream str;
                str << "Threading of #";
                str << (i+1);
                profiler::start("Raytracer Results", str.str());
                int j = 0;
                while (true) {
                    std::vector<int> func;
                    // get the function for the quadrant
                    queueSync.lock();
                    if (unprocessedQuads->empty()) {
                        queueSync.unlock();
                        break;
                    }
                    func = unprocessedQuads->front();
                    unprocessedQuads->pop();
                    queueSync.unlock();
                    // the run it
                    for (int kx = 0; kx <= func[0]; kx++) {
                        for (int ky = 0; ky < func[1]; ky++) {
                            try {
                                int x = func[2] + kx;
                                int y = func[3] + ky;
                                Raytracing::Vec4 color;
                                // TODO: profile for speed;
                                for (int s = 0; s < raysPerPixel; s++) {
                                    // simulate anti aliasing by generating rays with very slight random directions
                                    color = color + raycast(camera.projectRay(x + rnd.getDouble(), y + rnd.getDouble()));
                                }
                                PRECISION_TYPE sf = 1.0 / raysPerPixel;
                                // apply pixel color with gamma correction
                                image.setPixelColor(x, y, {std::sqrt(sf * color.r()), std::sqrt(sf * color.g()), std::sqrt(sf * color.b())});
                                if (*haltExecution || *haltRaytracing)
                                    return;
                                while (*pauseRaytracing) // sleep for 1/60th of a second, or about 1 frame.
                                    std::this_thread::sleep_for(std::chrono::milliseconds(16));
                            } catch (std::exception& error) {
                                flog << "Possibly fatal error in the multithreaded raytracer!\n";
                                flog << error.what() << "\n";
                            }
                        }
                    }
                    j++;
                }
                finishedThreads++;
                profiler::end("Raytracer Results", str.str());
            }));
        }
    }
}
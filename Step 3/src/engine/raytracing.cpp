/*
 * Created by Brett Terpstra 6920201 on 16/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */
#include "engine/raytracing.h"
#include <queue>
#include <functional>
#include <engine/util/debug.h>

namespace Raytracing {

    Ray Camera::projectRay(PRECISION_TYPE x, PRECISION_TYPE y) {
        // transform the x and y to points from image coords to be inside the camera's viewport.
        double transformedX = (x / (image.getWidth() - 1));
        auto transformedY = (y / (image.getHeight() - 1));
        // then generate a ray which extends out from the camera position in the direction with respects to its position on the image
        return {position, imageOrigin + transformedX * horizontalAxis + transformedY * verticalAxis - position};
    }

    void Camera::lookAt(const Vec4& pos, const Vec4& lookAtPos, const Vec4& up) {
        // standard camera lookAt function
        auto w = (pos - lookAtPos).normalize();
        auto u = (Vec4::cross(up, w)).normalize();
        auto v = Vec4::cross(w, u);

        position = pos;
        horizontalAxis = viewportWidth * u;
        verticalAxis = viewportHeight * v;
        imageOrigin = position - horizontalAxis/2 - verticalAxis/2 - w;
    }

    void Camera::setRotation(const PRECISION_TYPE yaw, const PRECISION_TYPE pitch, const PRECISION_TYPE roll) {
        // TODO:
    }

    Vec4 Raycaster::raycast(const Ray& ray, int depth) {
        if (depth > maxBounceDepth)
            return {0,0,0};

        auto hit = world.checkIfHit(ray, 0.001, infinity);

        if (hit.first.hit) {
            auto object = hit.second;
            auto scatterResults = object->getMaterial()->scatter(ray, hit.first);
            // if the material scatters the ray, ie casts a new one,
            if (scatterResults.scattered) // attenuate the recursive raycast by the material's color
                return scatterResults.attenuationColor * raycast(scatterResults.newRay, depth + 1);
            //tlog << "Not scattered? " << object->getMaterial() << "\n";
            return {0,0,0};
        }

        // skybox color
        return {0.5, 0.7, 1.0};
    }
    void Raycaster::runSingle() {
        executors.push_back(new std::thread([this]() -> void {
            for (int i = 0; i < image.getWidth(); i++){
                for (int j = 0; j < image.getHeight(); j++){
                    Raytracing::Vec4 color;
                    // TODO: profile for speed;
                    for (int s = 0; s < raysPerPixel; s++){
                        // simulate anti aliasing by generating rays with very slight random directions
                        color = color + raycast(camera.projectRay(i + rnd.getDouble(), j + rnd.getDouble()), 0);
                    }
                    PRECISION_TYPE sf = 1.0 / raysPerPixel;
                    // apply pixel color with gamma correction
                    image.setPixelColor(i, j, {std::sqrt(sf * color.r()), std::sqrt(sf * color.g()), std::sqrt(sf * color.b())});
                }
            }
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
        auto divs = (double) int(std::log(t)/std::log(2));
        // now double the divs, splitting each quadrant into 4 sub-quadrants which we can queue
        // the reason to do this is that some of them will finish before others, and the now free threads can keep working
        // do it without a queue like this leads to a single thread critical path and isn't optimally efficient.
        divs *= 2; // 2 because two axis getting split makes 4 sub-quadrants
        std::queue<std::function<void()>> unprocessedQuads;
        
        for (int dx = 0; dx < divs; dx++){
            for (int dy = 0; dy < divs; dy++){
                unprocessedQuads.push([this, divs, dx, dy]() -> void {
                    for (int i = 0; i <= std::ceil(image.getWidth() / divs); i++){
                        for (int j = 0; j < std::ceil(image.getHeight() / divs); j++){
                            try {
                                int x = int(std::floor(image.getWidth() / divs) * dx) + i;
                                int y = int(std::floor(image.getHeight() / divs) * dy) + j;
                                Raytracing::Vec4 color;
                                // TODO: profile for speed;
                                for (int s = 0; s < raysPerPixel; s++) {
                                    // simulate anti aliasing by generating rays with very slight random directions
                                    color = color + raycast(camera.projectRay(x + rnd.getDouble(), y + rnd.getDouble()), 0);
                                }
                                PRECISION_TYPE sf = 1.0 / raysPerPixel;
                                // apply pixel color with gamma correction
                                image.setPixelColor(x, y, {std::sqrt(sf * color.r()), std::sqrt(sf * color.g()), std::sqrt(sf * color.b())});
                            } catch (std::exception& error){
                                flog << "Possibly fatal error in the multithreaded raytracer!\n";
                                flog << error.what() << "\n";
                            }
                        }
                    }
                });
            }
        }
        
        for (int i = 0; i < t; i++){
            executors.push_back(new std::thread([this, unprocessedQuads, i]() -> void {
                // run through all the quadrants
                std::stringstream str;
                str << "Threading of ";
                str << i;
                profiler profile(str.str());
                int j = 0;
                while (!unprocessedQuads.empty()){
                    std::stringstream st;
                    st << "Queue element #";
                    st << j;
                    profile.start(st.str());
                    std::function<void()> func;
                    // get the function for the quadrant
                    {
                        std::unique_lock<std::mutex> locked(queueSync);
                        func = unprocessedQuads.front();
                    }
                    // the run it
                    // which despite its size should take the longest here.
                    func();
                    profile.end(st.str());
                    j++;
                }
                profile.print();
                finishedThreads++;
            }));
        }
        
    }
}
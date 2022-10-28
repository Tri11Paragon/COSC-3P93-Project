/*
 * Created by Brett Terpstra 6920201 on 16/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 *
 * The general class for all things raytracing!
 */

#ifndef STEP_2_RAYTRACING_H
#define STEP_2_RAYTRACING_H

#include "engine/math/vectors.h"
#include "engine/image/image.h"
#include "engine/util/parser.h"
#include "world.h"

#include <utility>
#include <mutex>
#include <thread>
#include <queue>

namespace Raytracing {

    class Camera {
        private:
            /* Image details */
            const Image image;
            const PRECISION_TYPE aspectRatio;

            /* Camera details */
            PRECISION_TYPE viewportHeight;
            PRECISION_TYPE viewportWidth;
            PRECISION_TYPE focalLength = 1.0;

            Vec4 position{0, 0, 0};
            Vec4 horizontalAxis;
            Vec4 verticalAxis;
            Vec4 imageOrigin;
        public:
            Camera(PRECISION_TYPE fov, const Image& image): image(image),
                                                            aspectRatio(double(image.getWidth()) / double(image.getHeight())) {
                // scale the viewport height based on the camera's FOV
                viewportHeight = (2.0 * tan(degreeeToRadian(fov) / 2));
                // with must respect the aspect ratio of the image, otherwise we'd get funky results
                viewportWidth = (aspectRatio * viewportHeight);
                // horizontal direction from the camera. used to translate the camera
                horizontalAxis = (Vec4{viewportWidth, 0, 0, 0});
                // virtual direction, also used to translate the camera
                verticalAxis = (Vec4{0, viewportHeight, 0, 0});
                // lower left of the camera's view port. used to project our vectors from image space to world space
                imageOrigin = (position - horizontalAxis / 2 - verticalAxis / 2 - Vec4(0, 0, focalLength, 0));

                tlog << viewportHeight << "\n";
                tlog << viewportWidth << "\n";
                tlog << "\n";
                tlog << horizontalAxis << "\n";
                tlog << verticalAxis << "\n";
                tlog << imageOrigin << "\n";

            }

            Ray projectRay(PRECISION_TYPE x, PRECISION_TYPE y);
            // makes the camera look at the lookatpos from the position p, with respects to the up direction up. (set to 0,1,0)
            void lookAt(const Vec4& pos, const Vec4& lookAtPos, const Vec4& up);

            void setPosition(const Vec4& pos) { this->position = pos; }

            void setRotation(PRECISION_TYPE yaw, PRECISION_TYPE pitch, PRECISION_TYPE roll);

    };

    static Random rnd{-1, 1};

    class Raycaster {
        private:
            int maxBounceDepth = 50;
            int raysPerPixel = 50;

            Camera& camera;
            Image& image;
            World& world;
            
            std::vector<std::unique_ptr<std::thread>> executors {};
            // is the raytracer still running?
            bool stillRunning = true;
            unsigned int finishedThreads = 0;
            unsigned int system_threads = std::thread::hardware_concurrency();
            // yes this is actually the only sync we need between the threads
            // and compared to the actual runtime of the raytracing it's very small!
            std::mutex queueSync;
            std::queue<std::vector<int>>* unprocessedQuads = nullptr;

            Vec4 raycast(const Ray& ray);
        public:
            inline void updateRayInfo(int maxBounce, int perPixel){
                raysPerPixel = perPixel;
                maxBounceDepth = maxBounce;
            }
            inline void resetRayInfo(){
                raysPerPixel = 50;
                maxBounceDepth = 50;
            }
            inline static Vec4 randomUnitVector() {
                // there are two methods to generating a random unit sphere
                // one which is fast and approximate:
                auto v = Vec4(rnd.getDouble(), rnd.getDouble(), rnd.getDouble());
                return v.normalize();
                // and the one which generates an actual unit vector
                /*while (true) {
                    auto v = Vec4(rnd.getDouble(), rnd.getDouble(), rnd.getDouble());
                    if (v.lengthSquared() >= 1)
                        continue;
                    return v;
                }*/
                // the second creates better results but is 18% slower (better defined shadows)
                // likely due to not over generating unit vectors biased towards the corners
            }
            Raycaster(Camera& c, Image& i, World& world, const Parser& p): camera(c), image(i), world(world) {
                world.generateBVH();
            }
            void runSingle();
            void runMulti(unsigned int t);
            [[nodiscard]] inline bool areThreadsStillRunning() const {return finishedThreads == executors.size();}
            inline void join(){
                for (auto& p : executors)
                    p->join();
            }
            void deleteThreads(){
                for (auto& p : executors){
                    // wait for all threads to exit before trying to delete them.
                    try {
                        if (p->joinable())
                            p->join();
                    } catch (std::exception& e){}
                }
                // since executors contains the only reference to the unique_ptr it will be deleted automatically
                executors.clear();
            }
            ~Raycaster() {
                deleteThreads();
                delete(unprocessedQuads);
            }
    };

}
#endif //STEP_2_RAYTRACING_H

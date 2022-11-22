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
            
            const PRECISION_TYPE NEAR_PLANE = 0.1;
            const PRECISION_TYPE FAR_PLANE = 500;
            PRECISION_TYPE tanFovHalf;
            PRECISION_TYPE frustumLength;

            Vec4 position{0, 0, 0};
            Vec4 horizontalAxis;
            Vec4 verticalAxis;
            Vec4 imageOrigin;
            
            Vec4 up {0, 1, 0};
            
        public:
            Camera(PRECISION_TYPE fov, const Image& image): image(image),
                                                            aspectRatio(double(image.getWidth()) / double(image.getHeight())) {
                // scale the viewport height based on the camera's FOV
                tanFovHalf = tan(degreeeToRadian(fov) / 2);
                viewportHeight = (2.0 * tanFovHalf);
                // with must respect the aspect ratio of the image, otherwise we'd get funky results
                viewportWidth = (aspectRatio * viewportHeight);
                frustumLength = FAR_PLANE - NEAR_PLANE;
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

            void setPosition(const Vec4& pos) { this->position = pos; }

            void setRotation(PRECISION_TYPE yaw, PRECISION_TYPE pitch);
            
            // the follow utility functions are actually taking forever to get right
            // I can't tell if my projection calculation is off or the view calc?
            // got to install GLM to test which function works and which does. Maybe they are both bad. or Maybe it's my matrix impl
            // or maybe the whole rendering stack sucks
            [[nodiscard]] Mat4x4 project() const {
                Mat4x4 project {emptyMatrix};
                
                // this should be all it takes to create a mostly correct projection matrix
                project.m00(float(1.0 / (aspectRatio * tanFovHalf)));
                project.m11(float(1.0 / tanFovHalf));
                project.m22(float(-((FAR_PLANE + NEAR_PLANE) / frustumLength)));
                // this has been transposed
                project.m32(-1);
                project.m23(float(-((2 * NEAR_PLANE * FAR_PLANE) / frustumLength)));
                //project.m33(0);
                
                return project;
                // use GLM to debug issues with ^
                //glm::mat4 projectG = glm::perspective(glm::radians(90.0f), (float)aspectRatio, 0.1f, (float)1000);
                //return Mat4x4{projectG};
            }
            [[nodiscard]] Mat4x4 view(const Vec4& lookAtPos) const {
                Mat4x4 view;
                
                auto w = (position - lookAtPos).normalize(); // forward
                auto u = (Vec4::cross(up, w)).normalize(); // right
                auto v = Vec4::cross(w, u); // up
                
                view.m00(float(w.x()));
                view.m01(float(w.y()));
                view.m02(float(w.z()));
                view.m03(float(w.w()));
    
                view.m10(float(u.x()));
                view.m11(float(u.y()));
                view.m12(float(u.z()));
                view.m13(float(u.w()));
    
                view.m20(float(v.x()));
                view.m21(float(v.y()));
                view.m22(float(v.z()));
                view.m23(float(v.w()));
                
                // view matrix are inverted, dot product to simulate translate matrix multiplication
                view.m30(-float(Vec4::dot(u, position)));
                view.m31(-float(Vec4::dot(v, position)));
                view.m32(-float(Vec4::dot(w, position)));
                view.m33(1);
                
                return view;
            }
            Mat4x4 view(PRECISION_TYPE yaw, PRECISION_TYPE pitch);

            [[nodiscard]] inline Vec4 getPosition() const {return position;};
        
            // the camera's position must be set with setPosition(Vec4);
            // uses an internal up vector, assumed to be {0, 1, 0}
            // will make the camera look at provided position with respects to the current camera position.
            void lookAt(const Vec4& lookAtPos);
    };

    static Random rnd{-1.0, 1.0};

    struct RaycasterImageBounds {
        int width,height, x,y;
    };

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
            std::queue<RaycasterImageBounds>* unprocessedQuads = nullptr;

            Vec4 raycasti(const Ray& ray, int depth);
            Vec4 raycast(const Ray& ray);
            void runRaycastingAlgorithm(RaycasterImageBounds imageBounds, int loopX, int loopY);
            void runSTDThread(int threads);
            void runOpenMP(int threads);
            void runMPI(int threads);
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
            void run(bool multithreaded, int threads = 0);
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

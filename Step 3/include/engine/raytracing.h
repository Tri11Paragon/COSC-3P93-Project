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
            
            Vec4 up{0, 1, 0};
        
        public:
            Camera(PRECISION_TYPE fov, const Image& image):
                    image(image),
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
            
            /**
             * Projects an xy coord into world space
             * @param x image x coord
             * @param y image y coord
             * @return a Ray projected from camera position out into the world based on the relative position in the image.
             */
            Ray projectRay(PRECISION_TYPE x, PRECISION_TYPE y);
            
            void setPosition(const Vec4& pos) { this->position = pos; }
            
            /**
             * Creates a projection matrix for use in the OpenGL pipeline.
             * @return Mat4x4 containing a standard perspective projection matrix
             */
            [[nodiscard]] inline Mat4x4 project() const {
                Mat4x4 project{emptyMatrix};
                
                // this should be all it takes to create a mostly correct projection matrix
                // the values are transposed because my matrix implementation is terrible.
                // This is set up in such a way that it is 1:1 with the CPU ray projection. Meaning when you move the camera in "Debug" mode,
                // the rays will be projected from that position and camera look direction.
                project.m00(float(1.0 / (aspectRatio * tanFovHalf)));
                project.m11(float(1.0 / tanFovHalf));
                project.m22(float(-((FAR_PLANE + NEAR_PLANE) / frustumLength)));
                project.m32(-1);
                project.m23(float(-((2 * NEAR_PLANE * FAR_PLANE) / frustumLength)));
                //project.m33(0);
                
                return project;
                // use GLM to debug issues with ^
                //glm::mat4 projectG = glm::perspective(glm::radians(90.0f), (float)aspectRatio, 0.1f, (float)1000);
                //return Mat4x4{projectG};
            }
            
            /**
             * Creates a view matrix containing the camera rotation and inverse position.
             * the view matrix is used to transform world coordinates into camera space,
             * which can than be transformed into screen space using the projection matrix.
             * @param yaw yaw of the camera
             * @param pitch pitch of the camera
             * @param roll NOT SUPPORTED
             * @return Mat4x4 containing rotation in the first 3x3 values and -position in the last column
             */
            Mat4x4 view(PRECISION_TYPE yaw, PRECISION_TYPE pitch);
            
            [[nodiscard]] inline Vec4 getPosition() const { return position; };
            
            [[nodiscard]] inline Vec4 getImageOrigin() const { return imageOrigin; }
            
            [[nodiscard]] inline Vec4 getHorizontalAxis() const { return horizontalAxis; }
            
            [[nodiscard]] inline Vec4 getVerticalAxis() const { return verticalAxis; }
            
            // the camera's position must be set with setPosition(Vec4);
            // uses an internal up vector, assumed to be {0, 1, 0}
            // will make the camera look at provided position with respects to the current camera position.
            // TODO: update the view matrix. Requires that the view matrix be stored in the camera.
            void lookAt(const Vec4& lookAtPos);
    };
    
    static Random rnd{-1.0, 1.0};
    
    struct RayCasterImageBounds {
        int width, height, x, y;
    };
    
    class RayCaster {
        private:
            const unsigned int system_threads = std::thread::hardware_concurrency();
            
            int maxBounceDepth;
            int raysPerPixel;
            unsigned int finishedThreads = 0;
            
            Camera& camera;
            Image& image;
            World& world;
            
            // yes this is actually the only sync we need between the threads
            // and compared to the actual runtime of the raytracing it's very small!
            std::mutex queueSync;
            // the queue containing the image bounds to be rendered.
            std::queue<RayCasterImageBounds>* unprocessedQuads = nullptr;
            std::vector<std::unique_ptr<std::thread>> executors{};
            
            /**
             * Does the actual ray casting algorithm. Simulates up to maxBounceDepth ray depth.
             * @param ray ray to begin with
             * @return the overall average color of the ray
             */
            Vec4 raycast(const Ray& ray);
            
            /**
             *
             * @param imageBounds bounds to work on
             * @param loopX the current x position to work on, between 0 and imageBounds.width
             * @param loopY the current y position to work on, between 0 and imageBounds.height
             */
            void runRaycastingAlgorithm(RayCasterImageBounds imageBounds, int loopX, int loopY);
            
            /**
             * Creates the queue with the provided bounds
             */
            void setupQueue(const std::vector<RayCasterImageBounds>& bounds);
        
        public:
            RayCaster(Camera& c, Image& i, World& world, Parser& p):
                    camera(c), image(i), world(world) {
                world.generateBVH();
                maxBounceDepth = std::stoi(p.getOptionValue("--maxRayDepth"));
                raysPerPixel = std::stoi(p.getOptionValue("--raysPerPixel"));
            }
            
            inline void updateRayInfo(int maxBounce, int perPixel) {
                raysPerPixel = perPixel;
                maxBounceDepth = maxBounce;
            }
            
            /**
             * divides the screen into image bounds
             * @param threads number of threads that will determine how many cuts to the screen is required
             * @return a list of bounds
             */
            std::vector<RayCasterImageBounds> partitionScreen(int threads = -1);
            
            /**
             * Updates the thread value based on conditions, used for setting up the actual threads for execution.
             * @param threads reference to the value which will be updated.
             */
            inline void updateThreadValue(int& threads) const {
                if (threads < 0 || threads == 1)
                    threads = 1;
                else {
                    if (threads == 0)
                        threads = (int) system_threads;
                }
            }
            
            /**
             * Creates a random vector in the unit sphere.
             */
            inline static Vec4 randomUnitVector() {
                return Vec4(rnd.getDouble(), rnd.getDouble(), rnd.getDouble()).normalize();
            }
            
            /**
             * Runs the std::thread implementation
             * @param threads number of threads to use
             */
            void runSTDThread(int threads = -1);
            
            /**
             * Runs the OpenMP implementation
             * @param threads number of threads to use
             */
            void runOpenMP(int threads = -1);
            
            /**
             * ran by MPI
             * @param bounds bounds that get processed by this process
             */
            void runMPI(std::queue<RayCasterImageBounds> bounds);
            
            /**
             * Blocking call that waits for all the threads to finish exeuction
             */
            inline void join() {
                for (auto& p : executors)
                    p->join();
            }
            
            /**
             * Joins all joinable threads and clears the thread list.
             */
            void deleteThreads() {
                for (auto& p : executors) {
                    // wait for all threads to exit before trying to delete them.
                    try {
                        if (p->joinable())
                            p->join();
                    } catch (std::exception& e) {}
                }
                // since executors contains the only reference to the unique_ptr it will be deleted automatically
                executors.clear();
            }
            
            ~RayCaster() {
                deleteThreads();
                delete (unprocessedQuads);
            }
    };
    
}
#endif //STEP_2_RAYTRACING_H

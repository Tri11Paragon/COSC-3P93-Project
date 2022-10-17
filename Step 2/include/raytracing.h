/*
 * Created by Brett Terpstra 6920201 on 16/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 *
 * The general class for all things raytracing!
 */

#ifndef STEP_2_RAYTRACING_H
#define STEP_2_RAYTRACING_H

#include <math/vectors.h>
#include <image/image.h>
#include <util/parser.h>
#include <world.h>

#include <utility>

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

            vec4 position{0, 0, 0};
            vec4 horizontalAxis;
            vec4 verticalAxis;
            vec4 imageOrigin;
        public:
            Camera(PRECISION_TYPE fov, const Image& image): image(image),
                                                            aspectRatio(double(image.getWidth()) / double(image.getHeight())) {
                viewportHeight = (2.0 * tan(degreeeToRadian(fov) / 2));
                viewportWidth = (aspectRatio * viewportHeight);
                horizontalAxis = (vec4{viewportWidth, 0, 0, 0});
                verticalAxis = (vec4{0, viewportHeight, 0, 0});
                imageOrigin = (position - horizontalAxis / 2 - verticalAxis / 2 - vec4(0, 0, focalLength, 0));

                tlog << viewportHeight << "\n";
                tlog << viewportWidth << "\n";
                tlog << "\n";
                tlog << horizontalAxis << "\n";
                tlog << verticalAxis << "\n";
                tlog << imageOrigin << "\n";

            }

            Ray projectRay(PRECISION_TYPE x, PRECISION_TYPE y);
            // makes the camera look at the lookatpos from the position p, with respects to the up direction up. (set to 0,1,0)
            void lookAt(const vec4& pos, const vec4& lookAtPos, const vec4& up);

            void setPosition(const vec4& pos) { this->position = pos; }

            void setRotation(PRECISION_TYPE yaw, PRECISION_TYPE pitch, PRECISION_TYPE roll);

    };

    class Raycaster {
        private:
            const int maxBounceDepth = 50;
            const int raysPerPixel = 50;

            Camera& camera;
            Image& image;
            World& world;
            Random rnd{-1, 1};

            vec4 randomUnitVector() {
                // there are two methods to generating a random unit sphere
                // one which is fast and approximate:
                //auto v = vec4(rnd.getDouble(), rnd.getDouble(), rnd.getDouble());
                //return v.normalize();
                // and the one which generates an actual unit vector
                while (true) {
                    auto v = vec4(rnd.getDouble(), rnd.getDouble(), rnd.getDouble());
                    if (v.lengthSquared() >= 1)
                        continue;
                    return v;
                }
                // the second creates better results but is 18% slower (better defined shadows)
                // likely due to not over generating unit vectors biased towards the corners
            }
            // unused but provides another method of diffuse rendering
            vec4 randomUnitHemisphere(const vec4& normal){
                vec4 v = randomUnitVector().normalize();
                if (vec4::dot(v, normal) > 0.0)
                    return v;
                else
                    return -v;
            }

            vec4 raycast(const Ray& ray, int depth);
        public:
            Raycaster(Camera& c, Image& i, World& world, const Parser& p): camera(c), image(i), world(world) {}

            void run();

    };

}
#endif //STEP_2_RAYTRACING_H

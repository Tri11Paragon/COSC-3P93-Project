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

namespace Raytracing {

    class Ray {
        private:
            // the starting point for our ray
            vec4 start;
            // and the direction it is currently traveling
            vec4 direction;
        public:
            Ray(const vec4& start, const vec4& direction): start(start), direction(direction) {}

            vec4 getStartingPoint() const { return start; }

            vec4 getDirection() const { return direction; }

            // returns a point along the ray, extended away from start by the length.
            inline vec4 along(PRECISION_TYPE length) const { return start + length * direction; }

    };

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
            void setPosition(const vec4& pos) {this->position = pos;}
            void setRotation(PRECISION_TYPE yaw, PRECISION_TYPE pitch, PRECISION_TYPE roll);

    };

    class Object {
        public:
            struct HitData {
                // all the other values only matter if this is true
                bool hit{false};
                // the hit point on the object
                vec4 hitPoint{};
                // the normal of that hit point
                vec4 normal{};
                // the length of the vector from its origin in its direction.
                PRECISION_TYPE length{0};
            };
            // return true if the ray intersects with this object, only between min and max
            virtual HitData checkIfHit(const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max) = 0;
    };

    class SphereObject : public Object {
        private:
            vec4 position;
            PRECISION_TYPE radius;

        public:
            SphereObject(const vec4& position, PRECISION_TYPE radius): position(position), radius(radius) {}

            virtual HitData checkIfHit(const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max);
    };

}
#endif //STEP_2_RAYTRACING_H

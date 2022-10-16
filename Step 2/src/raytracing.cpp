/*
 * Created by Brett Terpstra 6920201 on 16/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */
#include <raytracing.h>

namespace Raytracing {

    Ray Camera::projectRay(PRECISION_TYPE x, PRECISION_TYPE y) {
        // transform the x and y to points from image coords to be inside the camera's viewport.
        double transformedX = (x / (image.getWidth() - 1));
        auto transformedY = (y / (image.getHeight() - 1));
        // then generate a ray which extends out from the camera position in the direction with respects to its position on the image
        return {position, imageOrigin + transformedX * horizontalAxis + transformedY * verticalAxis - position};
    }

    void Camera::lookAt(const vec4& pos, const vec4& lookAtPos, const vec4& up) {
        // standard camera lookAt function
        auto w = (pos - lookAtPos).normalize();
        auto u = (vec4::cross(up, w)).normalize();
        auto v = vec4::cross(w, u);

        position = pos;
        horizontalAxis = viewportWidth * u;
        verticalAxis = viewportHeight * v;
        imageOrigin = position - horizontalAxis/2 - verticalAxis/2 - w;
    }

    void Camera::setRotation(const PRECISION_TYPE yaw, const PRECISION_TYPE pitch, const PRECISION_TYPE roll) {
        // TODO:
    }

    Object::HitData SphereObject::checkIfHit(const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max) {
        PRECISION_TYPE radiusSquared = radius * radius;
        // move the ray to be with respects to the sphere
        vec4 RayWRTSphere = ray.getStartingPoint() - position;
        // now determine the discriminant for the quadratic formula for the function of line sphere intercept
        PRECISION_TYPE a = ray.getDirection().lengthSquared();
        PRECISION_TYPE b = Raytracing::vec4::dot(RayWRTSphere, ray.getDirection());
        PRECISION_TYPE c = RayWRTSphere.lengthSquared() - radiusSquared;
        // > 0: the hit has two roots, meaning we hit both sides of the sphere
        // = 0: the ray has one root, we hit the edge of the sphere
        // < 0: ray isn't inside the sphere.
        PRECISION_TYPE discriminant = b * b - (a * c);

        // < 0: ray isn't inside the sphere. Don't need to bother calculating the roots.
        if (discriminant < 0)
            return {false, vec4(), vec4(), 0};

        // now we have to find the root which exists inside our range [min,max]
        auto root = (-b - sqrt(discriminant)) / a;
        // if the first root isn't in our range
        if (root < min || root > max) {
            // check the second root
            root = (-b + sqrt(discriminant)) / a;
            if (root < min || root > max) {
                // if the second isn't in the range then we also must return false.
                return {false, vec4(), vec4(), 0};
            }
        }
        // the hit point is where the ray is when extended to the root
        auto RayAtRoot = ray.along(root);
        // The normal of a sphere is just the point of the hit minus the center position
        auto normal = (RayAtRoot - position).normalize();
        return {true, RayAtRoot, normal, root};
    }
}
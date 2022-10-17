/*
 * Created by Brett Terpstra 6920201 on 16/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */
#include <raytracing.h>
#include <queue>

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

    void Raycaster::run() {
        for (int i = 0; i < image.getWidth(); i++){
            for (int j = 0; j < image.getHeight(); j++){
                Raytracing::vec4 color;
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
    }

    vec4 Raycaster::raycast(const Ray& ray, int depth) {
        if (depth > maxBounceDepth)
            return {0,0,0};

        auto hit = world.checkIfHit(ray, 0.001, infinity);

        if (hit.first.hit) {
            auto object = hit.second;
            auto scatterResults = object->getMaterial()->scatter(ray, hit.first);
            // if the material scatters the ray, ie casts a new one,
            if (scatterResults.scattered) // attenuate the recursive raycast by the material's color
                return scatterResults.attenuationColor * raycast(scatterResults.newRay, depth + 1);
            return {0,0,0};
        }

        vec4 dir = ray.getDirection().normalize();
        auto t = 0.5f * (dir.y() + 1.0);
        return (1.0 - t) * vec4(1.0, 1.0, 1.0) + t * vec4(0.5, 0.7, 1.0);
    }
}
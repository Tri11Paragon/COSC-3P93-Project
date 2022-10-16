/*
 * Created by Brett Terpstra 6920201 on 16/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */
#include <world.h>

namespace Raytracing {

    World::~World() {
        for (auto* p : objects)
            delete(p);
    }

    Object::HitData World::checkIfHit(const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max) const {
        auto hResult = Object::HitData{false, vec4(), vec4(), max};
        for (auto* obj : objects){
            // check up to the point of the last closest hit,
            // will give the closest object's hit result
            auto cResult = obj->checkIfHit(ray, min, hResult.length);
            if (cResult.hit)
                hResult = cResult;
        }
        return hResult;
    }
}
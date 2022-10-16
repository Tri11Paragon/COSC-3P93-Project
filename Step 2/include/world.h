/*
 * Created by Brett Terpstra 6920201 on 16/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */

#ifndef STEP_2_WORLD_H
#define STEP_2_WORLD_H

#include <util/std.h>
#include <raytracing.h>

namespace Raytracing {
class World {
    private:
        // store all the objects in the world,
        std::vector<Object*> objects;
        /*TODO: create a kd-tree or bvh version to store the objects
         * this way we can easily tell if a ray is near and object or not
         * saving on computation
         */
    public:
        World() {}
        inline void add(Object* object) {objects.push_back(object);}
        [[nodiscard]] virtual Object::HitData checkIfHit(const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max) const;
        ~World();

};
}

#endif //STEP_2_WORLD_H

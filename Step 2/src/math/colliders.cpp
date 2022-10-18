/*
 * Created by Brett Terpstra 6920201 on 17/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */
#include <math/colliders.h>

namespace Raytracing {

    PRECISION_TYPE AABB::longestDistanceFromCenter() const {
        vec4 center = getCenter();
        PRECISION_TYPE maxX = std::abs(max.x() - center.x());
        PRECISION_TYPE minX = std::abs(min.x() - center.x());
        PRECISION_TYPE maxY = std::abs(max.y() - center.y());
        PRECISION_TYPE minY = std::abs(min.y() - center.y());
        PRECISION_TYPE maxZ = std::abs(max.z() - center.z());
        PRECISION_TYPE minZ = std::abs(min.z() - center.z());
        return std::max(maxX, std::max(minX, std::max(maxY, std::max(minY, std::max(maxZ, minZ)))));
    }

    PRECISION_TYPE AABB::avgDistanceFromCenter() const {
        vec4 center = getCenter();
        PRECISION_TYPE maxX = std::abs(max.x() - center.x());
        PRECISION_TYPE minX = std::abs(min.x() - center.x());
        PRECISION_TYPE maxY = std::abs(max.y() - center.y());
        PRECISION_TYPE minY = std::abs(min.y() - center.y());
        PRECISION_TYPE maxZ = std::abs(max.z() - center.z());
        PRECISION_TYPE minZ = std::abs(min.z() - center.z());
        maxX *= maxX;
        minX *= minX;
        maxY *= maxY;
        minY *= minY;
        maxZ *= maxZ;
        minZ *= minZ;
        return std::sqrt(maxX + minX + maxY + minY + maxZ + minZ);
    }

    int AABB::longestAxis() const {
        PRECISION_TYPE X = std::abs(max.x() - min.x());
        PRECISION_TYPE Y = std::abs(max.y() - min.y());
        PRECISION_TYPE Z = std::abs(max.z() - min.z());
        return X > Y && X > Z ? 0 : Y > Z ? 1 : 2;
    }

    PRECISION_TYPE AABB::longestAxisLength() const {
        PRECISION_TYPE X = std::abs(max.x() - min.x());
        PRECISION_TYPE Y = std::abs(max.y() - min.y());
        PRECISION_TYPE Z = std::abs(max.z() - min.z());
        return X > Y && X > Z ? X : Y > Z ? Y : Z;
    }

    std::vector<AABB> AABB::splitByLongestAxis() const {
        PRECISION_TYPE X = std::abs(max.x() - min.x());
        PRECISION_TYPE Y = std::abs(max.y() - min.y());
        PRECISION_TYPE Z = std::abs(max.z() - min.z());
        if (X > Y && X > Z) {
            PRECISION_TYPE x2 = X/2.0;
            
        } else if (Y > Z) {

        } else {

        }
    }
}
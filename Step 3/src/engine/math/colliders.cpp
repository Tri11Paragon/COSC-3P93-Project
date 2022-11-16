/*
 * Created by Brett Terpstra 6920201 on 17/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */
#include "engine/math/colliders.h"

namespace Raytracing {

    PRECISION_TYPE AABB::longestDistanceFromCenter() const {
        Vec4 center = getCenter();
        PRECISION_TYPE maxX = std::abs(max.x() - center.x());
        PRECISION_TYPE minX = std::abs(min.x() - center.x());
        PRECISION_TYPE maxY = std::abs(max.y() - center.y());
        PRECISION_TYPE minY = std::abs(min.y() - center.y());
        PRECISION_TYPE maxZ = std::abs(max.z() - center.z());
        PRECISION_TYPE minZ = std::abs(min.z() - center.z());
        return std::max(maxX, std::max(minX, std::max(maxY, std::max(minY, std::max(maxZ, minZ)))));
    }

    PRECISION_TYPE AABB::avgDistanceFromCenter() const {
        Vec4 center = getCenter();
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

    std::pair<AABB, AABB> AABB::splitByLongestAxis() {
        PRECISION_TYPE X = std::abs(max.x() - min.x());
        PRECISION_TYPE X2 = X/2;
        PRECISION_TYPE Y = std::abs(max.y() - min.y());
        PRECISION_TYPE Y2 = Y/2;
        PRECISION_TYPE Z = std::abs(max.z() - min.z());
        PRECISION_TYPE Z2 = Z/2;
        // return the new split AABBs based on the calculated max lengths, but only in their respective axis.
        if (X > Y && X > Z) {
            return {{min.x(), min.y(), min.z(), max.x()-X2, max.y(), max.z()},
                    // start the second AABB at the end of the first AABB.
                    {min.x()+X2, min.y(), min.z(), max.x(), max.y(), max.z()}};
        } else if (Y > Z) {
            return {{min.x(), min.y(), min.z(), max.x(), max.y()-Y2, max.z()}, {min.x(), min.y()+Y2, min.z(), max.x(), max.y(), max.z()}};
        } else {
            return {{min.x(), min.y(), min.z(), max.x(), max.y(), max.z()-Z2}, {min.x(), min.y(), min.z()+Z2, max.x(), max.y(), max.z()}};
        }
    }

    /*
     * Sources for designing these various algorithms
     * TODO: test these methods for performance
     * https://www.realtimerendering.com/intersections.html
     * https://web.archive.org/web/20090803054252/http://tog.acm.org/resources/GraphicsGems/gems/RayBox.c
     * https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection
     * https://tavianator.com/2011/ray_box.html
     * https://medium.com/@bromanz/another-view-on-the-classic-ray-aabb-intersection-algorithm-for-bvh-traversal-41125138b525
     */

    AABBHitData AABB::simpleSlabRayAABBMethod(const Ray& ray, PRECISION_TYPE tmin, PRECISION_TYPE tmax){
        // branch less design
        // adapted from 2d to fit our 3d scene.
        // (turns out this is actually a pretty standard design, but could use some optimization)
        PRECISION_TYPE tx1 = (min.x() - ray.getStartingPoint().x())*ray.getInverseDirection().x();
        PRECISION_TYPE tx2 = (max.x() - ray.getStartingPoint().x())*ray.getInverseDirection().x();

        tmin = std::max(tmin, std::min(tx1, tx2));
        tmax = std::min(tmax, std::max(tx1, tx2));

        PRECISION_TYPE ty1 = (min.y() - ray.getStartingPoint().y())*ray.getInverseDirection().y();
        PRECISION_TYPE ty2 = (max.y() - ray.getStartingPoint().y())*ray.getInverseDirection().y();

        tmin = std::max(tmin, std::min(ty1, ty2));
        tmax = std::min(tmax, std::max(ty1, ty2));

        PRECISION_TYPE tz1 = (min.z() - ray.getStartingPoint().z())*ray.getInverseDirection().z();
        PRECISION_TYPE tz2 = (max.z() - ray.getStartingPoint().z())*ray.getInverseDirection().z();

        tmin = std::max(tmin, std::min(tz1, tz2));
        tmax = std::min(tmax, std::max(tz1, tz2));
        
        tmin = std::max(tmin, 0.0);
        
        // TODO: nans?
        //tlog << "TMin: " << tmin << " TMax: " << tmax << " Case: " << (tmax > tmin) << "\n";
        AABBHitData data{};
        data.hit = tmax > tmin;
        data.tMin = tmin;
        data.tMax = tmax;
        return data;
    }
    
    AABBHitData AABB::intersects(const Ray& ray, PRECISION_TYPE tmin, PRECISION_TYPE tmax) {
        return simpleSlabRayAABBMethod(ray, tmin, tmax);
    }
    
    std::pair<AABB, AABB> AABB::splitAlongAxis(AABBAxis axis) {
        // return the new split AABBs based on the calculated max lengths, but only in their respective axis.
        if (axis == X){
            PRECISION_TYPE X = std::abs(max.x() - min.x());
            PRECISION_TYPE X2 = X/2;
            // end the first at half the parent.
            return {{min.x(), min.y(), min.z(), max.x()-X2, max.y(), max.z()},
                    // start the second AABB at the end of the first AABB.
                    {min.x()+X2, min.y(), min.z(), max.x(), max.y(), max.z()}};
        } else if (axis == Y) {
            PRECISION_TYPE Y = std::abs(max.y() - min.y());
            PRECISION_TYPE Y2 = Y/2;
            return {{min.x(), min.y(), min.z(), max.x(), max.y()-Y2, max.z()}, {min.x(), min.y()+Y2, min.z(), max.x(), max.y(), max.z()}};
        } else {
            PRECISION_TYPE Z = std::abs(max.z() - min.z());
            PRECISION_TYPE Z2 = Z/2;
            return {{min.x(), min.y(), min.z(), max.x(), max.y(), max.z()-Z2}, {min.x(), min.y(), min.z()+Z2, max.x(), max.y(), max.z()}};
        }
    }
    Mat4x4 AABB::getTransform() const {
        Raytracing::Mat4x4 transform{};
        auto center = getCenter();
        transform.translate(center);
        auto xRadius = getXRadius(center) * 2;
        auto yRadius = getYRadius(center) * 2;
        auto zRadius = getZRadius(center) * 2;
        transform.scale(float(xRadius), float(yRadius), float(zRadius));
        return transform;
    }
}
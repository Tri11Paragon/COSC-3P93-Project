/*
 * Created by Brett Terpstra 6920201 on 17/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */

#ifndef STEP_2_COLLIDERS_H
#define STEP_2_COLLIDERS_H

#include "vectors.h"

namespace Raytracing {
    // 3D Axis Aligned Bounding Box for use in a BVH
    struct AABBHitData {
        bool hit;
        PRECISION_TYPE tMin, tMax;
    };
    
    enum AABBAxis {
        X = 0, Y = 1, Z = 2
    };
    
    /**
     * Simple minimal Axis Aligned Bounding Box implementation. Based on my C++ Game Engine's AABB which is based on my Java Game Engine's AABB
     * which is based on my Minecraft Clone v2's AABB which is based on a decompiled AABB class from Minecraft Beta 1.7.3
     */
    class AABB {
        protected:
            Vec4 min;
            Vec4 max;
        public:
            /**
             * Creates an empty AABB. Not useful for anything.
             */
            AABB(): min({0, 0, 0}), max({0, 0, 0}) {};
            
            /**
             * Creates an AABB using the provided min/max coords
             */
            AABB(PRECISION_TYPE minX, PRECISION_TYPE minY, PRECISION_TYPE minZ, PRECISION_TYPE maxX, PRECISION_TYPE maxY, PRECISION_TYPE maxZ):
                    min{minX, minY, minZ}, max{maxX, maxY, maxZ} {
            }
            
            /**
             * Creates an AABB using the provided min/max vector coords
             */
            AABB(const Vec4& min, const Vec4& max): min(min), max(max) {}
            
            /**
             * creates an AABB extending of size centered on x, y, z
             * @param size radius of the AABB
             */
            AABB(PRECISION_TYPE x, PRECISION_TYPE y, PRECISION_TYPE z, PRECISION_TYPE size):
                    min{x - size, y - size, z - size}, max{x + size, y + size, z + size} {
            }
            
            /**
             * translates the AABB to position x,y,z for world collision detection
             * This creates a copy AABB.
             * @return a copy of this AABB translated by Vec4{x,y,z}
             */
            [[nodiscard]] AABB translate(PRECISION_TYPE x, PRECISION_TYPE y, PRECISION_TYPE z) const {
                Vec4 pos = {x, y, z};
                return {min + pos, max + pos};
            }
            
            /**
             * Translates this AABB using a vector instead of individual coords
             * @return a copy of this AABB translated by vec
             */
            [[nodiscard]] AABB translate(const Vec4& vec) const {
                Vec4 pos = {vec.x(), vec.y(), vec.z()};
                return {min + pos, max + pos};
            }
            
            /**
             * returns an expanded version of this AABB is the other AABB is larger then this AABB
             * @param other other AABB to expand against
             * @return a AABB which includes this AABB and the other AABB. If this AABB is empty it will return other.
             */
            [[nodiscard]] AABB expand(const AABB& other) const {
                // terrible hack
                // a 0 init AABB was having issues when being "expanded" to a place which is larger
                // this should prevent that by side stepping the issue. Which is a TODO:
                if (isEmpty())
                    return other;
                PRECISION_TYPE minX = std::min(min.x(), other.min.x());
                PRECISION_TYPE minY = std::min(min.y(), other.min.y());
                PRECISION_TYPE minZ = std::min(min.z(), other.min.z());
                PRECISION_TYPE maxX = std::max(max.x(), other.max.x());
                PRECISION_TYPE maxY = std::max(max.y(), other.max.y());
                PRECISION_TYPE maxZ = std::max(max.z(), other.max.z());
                return {minX, minY, minZ, maxX, maxY, maxZ};
            }
            
            /**
             * Checks if this AABB interests with the AABB described in individual coordinates
             * @return true if intersection occurs
             */
            [[nodiscard]] inline bool intersects(
                    PRECISION_TYPE minX, PRECISION_TYPE minY, PRECISION_TYPE minZ, PRECISION_TYPE maxX, PRECISION_TYPE maxY, PRECISION_TYPE maxZ
            ) const {
                return min.x() <= maxX && max.x() >= minX && min.y() <= maxY && max.y() >= minY && min.z() <= maxZ && max.z() >= minZ;
            }
            
            /**
             * Checks if this AABB interests with the AABB described in vector coordinates
             * @return true if intersection occurs
             */
            [[nodiscard]] inline bool intersects(const Vec4& minV, const Vec4& maxV) const {
                return intersects(minV.x(), minV.y(), minV.z(), maxV.x(), maxV.y(), maxV.z());
            }
            
            /**
             * Checks if this AABB intersects with the other AABB
             * @return true if intersection
             */
            [[nodiscard]] inline bool intersects(const AABB& other) const {
                return intersects(other.min, other.max);
            }
            
            /**
             * Checks if the provided ray intersects with this AABB between tmin and tmax using an implementation defined method.
             * @return true if intersected
             */
            AABBHitData intersects(const Ray& ray, PRECISION_TYPE tmin, PRECISION_TYPE tmax);
            
            /**
             * Slap method of checking for intersection, see the comment in the cpp file. DO NOT USE OUTSIDE THIS CLASS.
             */
            AABBHitData simpleSlabRayAABBMethod(const Ray& ray, PRECISION_TYPE tmin, PRECISION_TYPE tmax);
            
            /**
             * Creates a transform matrix using information contained within the AABB, done to prevent repeating code.
             * @return a transform (model) matrix for this AABB.
             */
            [[nodiscard]] Mat4x4 getTransform() const;
            
            /**
             * @return the exact center of the AABB
             */
            [[nodiscard]] inline Vec4 getCenter() const {
                return {min.x() + (max.x() - min.x()) * 0.5, min.y() + (max.y() - min.y()) * 0.5, min.z() + (max.z() - min.z()) * 0.5};
            }
            
            /**
             * @return the longest axis distance from the center point of the AABB
             */
            [[nodiscard]] PRECISION_TYPE longestDistanceFromCenter() const;
            
            /**
             * 0 - x
             * 1 - y
             * 2 - z
             */
            [[nodiscard]] int longestAxis() const;
            
            /**
             * @return the complete length from min to max of the longest axis
             */
            [[nodiscard]] PRECISION_TYPE longestAxisLength() const;
            
            /**
             * @return a pair of AABB which are subsets of this AABB split by the longest axis of this AABB
             */
            [[nodiscard]] std::pair<AABB, AABB> splitByLongestAxis();
            
            /**
             * @param axis axis to split on
             * @return a pair of AABB which are subsets of this AABB split along the axis provided
             */
            [[nodiscard]] std::pair<AABB, AABB> splitAlongAxis(AABBAxis axis);
            
            /**
             * Splits the AABB by a rotating axis. Not recommended to use. Not thread safe across ALL AABB objects.
             */
            [[nodiscard]] std::pair<AABB, AABB> splitAlongAxis();
            
            /**
             * @return average distance to the center of this object, subject to axis bias (really small or really big)
             */
            [[nodiscard]] PRECISION_TYPE avgDistanceFromCenter() const;
            
            /**
             * Returns true if the min and max are equal, which tells us this AABB wasn't assigned
             * or was properly created. Either way it isn't responsible to use the AABB in said case.
             */
            [[nodiscard]] inline bool isEmpty() const { return min == max; }
            
            [[nodiscard]] Vec4 getMin() const { return min; }
            
            [[nodiscard]] Vec4 getMax() const { return max; }
            
            [[nodiscard]] inline PRECISION_TYPE getXRadius(const Vec4& center) const {
                return max.x() - center.x();
            }
            
            [[nodiscard]] inline PRECISION_TYPE getYRadius(const Vec4& center) const {
                return max.y() - center.y();
            }
            
            [[nodiscard]] inline PRECISION_TYPE getZRadius(const Vec4& center) const {
                return max.z() - center.z();
            }
            
            /*
             * Minecraft code below this
             */
            [[nodiscard]] inline bool isInside(PRECISION_TYPE x, PRECISION_TYPE y, PRECISION_TYPE z) const {
                return x >= min.x() && x <= max.x() && y >= min.y() && y <= max.y() && z >= min.z() && z <= max.z();
            }
            
            [[nodiscard]] inline bool intersectsWithYZ(PRECISION_TYPE y, PRECISION_TYPE z) const {
                return y >= min.y() && y <= max.y() && z >= min.z() && z <= max.z();
            }
            
            [[nodiscard]] inline bool intersectsWithXZ(PRECISION_TYPE x, PRECISION_TYPE z) const {
                return x >= min.x() && x <= max.x() && z >= min.z() && z <= max.z();
            }
            
            [[nodiscard]] inline bool intersectsWithXY(PRECISION_TYPE x, PRECISION_TYPE y) const {
                return x >= min.x() && x <= max.x() && y >= min.y() && y <= max.y();
            }
    };
    
    inline bool operator==(const AABB& a, const AABB& b) {
        const auto& aMax = a.getMax();
        const auto& aMin = a.getMin();
        const auto& bMax = b.getMax();
        const auto& bMin = b.getMin();
        return aMax == bMax && aMin == bMin;
    }
    
    inline std::ostream& operator<<(std::ostream& out, const AABB& v) {
        auto max = v.getMax();
        auto min = v.getMin();
        auto center = v.getCenter();
        return out << "AABB {\n\t min{" << min.x() << ", " << min.y() << ", " << min.z() << "},\n\t max{" << max.x() << ", " << max.y()
                   << ", " << max.z() << "},\n\t diff{" << max.x() - min.x() << ", " << max.y() - min.y() << ", " << max.z() - min.z() << "},\n\t "
                   << "center{" << center.x() << ", " << center.y() << ", " << center.z() << "},\n\t "
                   << "radi{" << v.getXRadius(center) << ", " << v.getYRadius(center) << ", " << v.getZRadius(center) << "}\n};\n";
    }
}

#endif //STEP_2_COLLIDERS_H

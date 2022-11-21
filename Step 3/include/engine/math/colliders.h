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
    
    class AABB {
        protected:
            Vec4 min;
            Vec4 max;
        public:
            AABB(): min({0,0,0}), max({0,0,0}) {};

            AABB(PRECISION_TYPE minX, PRECISION_TYPE minY, PRECISION_TYPE minZ, PRECISION_TYPE maxX, PRECISION_TYPE maxY, PRECISION_TYPE maxZ):
                    min{minX, minY, minZ}, max{maxX, maxY, maxZ} {
            }

            AABB(const Vec4& min, const Vec4& max): min(min), max(max) {}

            // creates an AABB extending of size centered on x, y, z
            AABB(PRECISION_TYPE x, PRECISION_TYPE y, PRECISION_TYPE z, PRECISION_TYPE size):
                    min{x - size, y - size, z - size}, max{x + size, y + size, z + size} {
            }

            // translates the AABB to position x,y,z for world collision detection
            [[nodiscard]] AABB translate(PRECISION_TYPE x, PRECISION_TYPE y, PRECISION_TYPE z) const {
                Vec4 pos = {x, y, z};
                return {min + pos, max + pos};
            }

            [[nodiscard]] AABB translate(const Vec4& vec) const {
                Vec4 pos = {vec.x(), vec.y(), vec.z()};
                return {min + pos, max + pos};
            }

            // returns an expanded version of this AABB is the other AABB is larger then this AABB
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

            [[nodiscard]] inline bool
            intersects(PRECISION_TYPE minX, PRECISION_TYPE minY, PRECISION_TYPE minZ, PRECISION_TYPE maxX, PRECISION_TYPE maxY,
                       PRECISION_TYPE maxZ) const {
                return min.x() <= maxX && max.x() >= minX && min.y() <= maxY && max.y() >= minY && min.z() <= maxZ && max.z() >= minZ;
            }

            [[nodiscard]] inline bool intersects(const Vec4& minV, const Vec4& maxV) const {
                return intersects(minV.x(), minV.y(), minV.z(), maxV.x(), maxV.y(), maxV.z());
            }

            [[nodiscard]] inline bool intersects(const AABB& other) const {
                return intersects(other.min, other.max);
            }
        
            AABBHitData intersects(const Ray& ray, PRECISION_TYPE tmin, PRECISION_TYPE tmax);
            AABBHitData simpleSlabRayAABBMethod(const Ray& ray, PRECISION_TYPE tmin, PRECISION_TYPE tmax);
            [[nodiscard]] Mat4x4 getTransform() const;

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

            [[nodiscard]] inline Vec4 getCenter() const {
                return {min.x() + (max.x() - min.x()) * 0.5, min.y() + (max.y() - min.y()) * 0.5, min.z() + (max.z() - min.z()) * 0.5};
            }

            [[nodiscard]] PRECISION_TYPE longestDistanceFromCenter() const;
            // 0 - x
            // 1 - y
            // 2 - z
            [[nodiscard]] int longestAxis() const;
            [[nodiscard]] PRECISION_TYPE longestAxisLength() const;
            [[nodiscard]] std::pair<AABB, AABB> splitByLongestAxis();
            [[nodiscard]] std::pair<AABB, AABB> splitAlongAxis(AABBAxis axis);
            [[nodiscard]] std::pair<AABB, AABB> splitAlongAxis();

            [[nodiscard]] PRECISION_TYPE avgDistanceFromCenter() const;

            // Returns true if the min and max are equal, which tells us this AABB wasn't assigned
            // or was properly created. Either way it isn't responsible to use the AABB in said case.
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
            

    };
    
    inline bool operator==(const AABB& a, const AABB& b){
        const auto& aMax = a.getMax();
        const auto& aMin = a.getMin();
        const auto& bMax = b.getMax();
        const auto& bMin = b.getMin();
        return aMax == bMax && aMin == bMin;
    }
    
    #ifndef USE_SIMD_CPU
    inline bool operator&=(const AABB& a, const AABB& b) {
        const auto& aMax = a.getMax();
        const auto& aMin = a.getMin();
        const auto& bMax = b.getMax();
        const auto& bMin = b.getMin();
        PRECISION_TYPE E = 0.5;
        return (aMax < bMax + E && aMax > bMax - E && aMin < bMin + E && aMin > bMin - E);
        return aMax == bMax && aMin == bMin;
    }
    #endif

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

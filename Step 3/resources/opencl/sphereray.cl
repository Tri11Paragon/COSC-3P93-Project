// these values get dynamically defined by the preprocessor 
#define maxTriangleCount 336
#define objectCount 6
#define imageWidth 800
#define imageHeight 600
/*#define imageOrigin (float3)(0,0,0)
#define horizontalAxis (float3)(0,0,0)
#define verticalAxis (float3)(0,0,0) 
#define cameraPosition (float3)(0,0,0) */

#define MAX_DEPTH 50
#define MAX_PER_PIXEL 50
#define PI 3.1415926535897

#include "randoms_git.cl"

struct Ray {
    // the starting point for our ray
    float3 start;
    // and the direction it is currently traveling
    float3 direction;
    float3 inverseDirection;
};

struct HitData {
    // the hit point on the object
    float3 hitPoint;
    // the normal of that hit point
    float3 normal;
    // the length of the vector from its origin in its direction.
    float length;
};

// required because for some reason OpenCL stores vectors in a really weird byte order??
// this prevents all of the graphical issues + allows us to assume the order *no platform dependance*
struct Vec {
    float x, y, z;
};

float3 randomVector(unsigned long seed){
    pcg6432_state state;
    pcg6432_seed(&state, seed);
    return ((float3)(pcg6432_float(state), pcg6432_float(state), pcg6432_float(state)) * 2) - 1;
}

float3 along(struct Ray ray, float length) { 
    return ray.start + length * ray.direction; 
}

float lengthSquared(float3 vec){
    return vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;
}

float magnitude(float3 vec){
    return sqrt(lengthSquared(vec));
}

struct Ray projectRay(__global struct Vec* cameraData, float x, float y){
    float transformedX = (x / (imageWidth - 1));
    float transformedY = (y / (imageHeight - 1));

    float3 cameraPosition = (float3)(cameraData[0].x, cameraData[0].y, cameraData[0].z);
    float3 verticalAxis = (float3)(cameraData[1].x, cameraData[1].y, cameraData[1].z);
    float3 horizontalAxis = (float3)(cameraData[2].x, cameraData[2].y, cameraData[2].z);
    float3 imageOrigin = (float3)(cameraData[3].x, cameraData[3].y, cameraData[3].z); 

    struct Ray ray;
    ray.start = cameraPosition;
    ray.direction = (imageOrigin + (transformedX * horizontalAxis) + (transformedY * verticalAxis)) - cameraPosition;
    ray.inverseDirection = 1.0f / ray.direction;

    return ray;
}

bool checkIfHit(struct HitData* data, struct Ray ray, float3 position, float radius, float min, float max){
    float radiusSquared = radius * radius;
    float3 rayWRTSphere = ray.start - position;
    // now determine the discriminant for the quadratic formula for the function of line sphere intercept
    float a = lengthSquared(ray.direction);
    float b = dot(rayWRTSphere, ray.direction);
    float c = lengthSquared(rayWRTSphere) - radiusSquared;
    // > 0: the hit has two roots, meaning we hit both sides of the sphere
    // = 0: the ray has one root, we hit the edge of the sphere
    // < 0: ray isn't inside the sphere.
    float discriminant = b * b - (a * c);
    
    // < 0: ray isn't inside the sphere. Don't need to bother calculating the roots.
    if (discriminant < 0) {
        return false;
    }
    
    // now we have to find the root which exists inside our range [min,max]
    float root = (-b - sqrt(discriminant)) / a;
    // if the first root isn't in our range
    if (root < min || root > max) {
        // check the second root
        root = (-b + sqrt(discriminant)) / a;
        if (root < min || root > max) {
            // if the second isn't in the range then we also must return false.
            return false;
        }
    }
    // the hit point is where the ray is when extended to the root
    float3 RayAtRoot = along(ray,root);
    // The normal of a sphere is just the point of the hit minus the center position
    float3 normal = (RayAtRoot - position) / radius;
    
    // have to invert the v since we have to invert the v again later due to triangles
    data->hitPoint = RayAtRoot;
    data->normal = normal;
    data->length = root;
    return true;
}

bool scatter(struct Ray* ray, struct HitData data, int currentDepth){
    const float EPSILON = 0.0000001f;
    int x = get_global_id(0);
    int y = get_global_id(1);
    unsigned long seed = x * y * currentDepth;
    float3 newRay = data.normal + normalize(randomVector(seed));

    // rays that are close to zero are liable to floating point precision errors
    if (newRay.x < EPSILON && newRay.y < EPSILON && newRay.z < EPSILON)
        newRay = data.normal;

    ray->start = data.hitPoint;
    ray->direction = newRay;
    ray->inverseDirection = 1/ray->direction;
    return true;
}

int checkWorldForIntersection(struct HitData* hit, struct Ray ray){
    const float4 positions[] = {
        (float4)(0, 1, -2, 1.0f),
        (float4)(0, -100.0f, 0, 100.0f),
        (float4)(0, 1, 0, 1.0f),
        (float4)(0, 1, 5, 1.0f),
        (float4)(10, 5, 5, 1.0f),
        
    };
    hit->length = 100000000.0f;
    int hasHit = 0;
    for (int i = 0; i < 5; i++){
        if (checkIfHit( hit, ray, (float3)(positions[i].x, positions[i].y, positions[i].z), positions[i].w, 0.001f, hit->length )){
            hasHit = i+1;
        }
    } 
    return hasHit;
}

float4 raycastI(struct Ray ray){
    const float4 colors[] = {
        (float4)(1.0f, 0.0f, 0.0f, 1.0f),
        (float4)(0.0f,1.0f,0.0f, 1.0f),
        (float4)(0.0f,0.0f,1.0f, 1.0f),
        (float4)(0.0f,1.0f,1.0f, 1.0f),
        (float4)(1.0f,0.0f,1.0f, 1.0f)
    };
    struct Ray localRay = ray;
    float4 localColor = (float4)(1.0f);
    
    for (int _ = 0; _ < MAX_DEPTH; _++){
        struct HitData hit;
        int hitIndex = checkWorldForIntersection(&hit, localRay);
        if ( hitIndex ){
            if (scatter(&localRay, hit, _)){
                localColor = localColor * colors[hitIndex-1];
            } else {
                localColor = (float4)(0.0,0.0,0.0,0.0);
                break;
            }
        } else {
            // since we didn't hit, we hit the sky.
            localColor = localColor * (float4)(0.5, 0.7, 1.0, 1.0);
            // if we don't hit we cannot keep looping.
            break;
        }
        localColor += localColor;
    }
    return localColor;
}

__kernel void raycast(__write_only image2d_t outputImage, __global unsigned char* objects, __global struct Vec* cameraData) {
    unsigned long currentByte = 0;
    int x = get_global_id(0);
    int y = get_global_id(1);
    float4 color = (float4)(0.0);

    for (int i = 0; i < MAX_PER_PIXEL; i++){
        pcg6432_state state;
        unsigned long seed = x * y * i;
        pcg6432_seed(&state, seed);
        color = color + raycastI(projectRay(cameraData, x + pcg6432_float(state), y + pcg6432_float(state)));
    }
    
    float scaleFactor = 1.0 / MAX_PER_PIXEL;
    write_imagef(outputImage, (int2)(x, y), (float4)(sqrt(color.x * scaleFactor), sqrt(color.y * scaleFactor), sqrt(color.z * scaleFactor), 1.0f));
    //pcg6432_state state;
    //unsigned long seed = x * y;
    //pcg6432_seed(&state, seed);
    //write_imagef(outputImage, (int2)(x, y), (float4)(randomVector(state), 1.0f));
}
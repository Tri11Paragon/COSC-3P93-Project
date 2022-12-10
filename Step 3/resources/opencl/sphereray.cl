// these values get dynamically defined by the preprocessor 
#define maxTriangleCount 336
#define objectCount 6
#define imageWidth 800
#define imageHeight 600
#define imageOrigin (float4)(0,0,0,0)
#define horizontalAxis (float4)(0,0,0,0)
#define verticalAxis (float4)(0,0,0,0) 
#define cameraPosition (float4)(0,0,0,0) 

#define MAX_DEPTH 500
#define MAX_PER_PIXEL 500
#define PI 3.1415926535897


struct Ray {
    // the starting point for our ray
    float4 start;
    // and the direction it is currently traveling
    float4 direction;
    float4 inverseDirection;
};

struct HitData {
    // the hit point on the object
    float4 hitPoint;
    // the normal of that hit point
    float4 normal;
    // the length of the vector from its origin in its direction.
    float length;
    // Texture UV Coords.
    float u, v;
};

struct ScatterResults {
    // returns true to recast the ray with the provided ray
    bool scattered;
    // the new ray to be cast if scattered
    struct Ray newRay;
    // the color of the material
    float4 attenuationColor;
};

float4 along(struct Ray ray, float length) { 
    return ray.start + length * ray.direction; 
}

float lengthSquared(float4 vec){
    return vec.x * vec.x + vec.y * vec.y + vec.z * vec.z + vec.w * vec.w;
}

float magnitude(float4 vec){
    return sqrt(lengthSquared(vec));
}

// // // reads one unsigned long of data from the objects buffer.
float4 getVector(__global unsigned char* buffer, unsigned long* currentByte){
     float4 val = *((global float4*)(buffer));
     *currentByte += sizeof(float4);
     return val;
}

struct Ray projectRay(float x, float y){
    float transformedX = (x / (imageWidth - 1));
    float transformedY = (y / (imageHeight - 1));

    struct Ray ray;
    ray.start = cameraPosition;
    ray.direction = imageOrigin + transformedX * horizontalAxis + transformedY * verticalAxis - cameraPosition;
    ray.inverseDirection = 1.0f / ray.direction;

    return ray;
}

bool checkIfHit(struct HitData* data, struct Ray ray, float4 position, float radius, float min, float max){
    float radiusSquared = radius * radius;
    float4 rayWRTSphere = ray.start - position;
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
    float4 RayAtRoot = along(ray,root);
    // The normal of a sphere is just the point of the hit minus the center position
    float4 normal = (RayAtRoot - position) / radius;
    
    // calculate the uv coords and normalize to [0, 1]
    float u = (atan2(-normal.z, normal.x) + PI) / (2 * PI);
    float v = acos(normal.y) / PI;
    // have to invert the v since we have to invert the v again later due to triangles
    data->hitPoint = RayAtRoot;
    data->normal = normal;
    data->length = root;
    data->u = u;
    data->v = 1.0 - v;
    return true;
}



__kernel void raycast(__write_only image2d_t outputImage, __global unsigned char* objects, __global unsigned char* randoms) {
    unsigned long currentByte = 0;
    int x = get_global_id(0);
    int y = get_global_id(1);
    float4 color = (float4)(0.0);


    struct HitData hitData1;
    if (checkIfHit(&hitData1, projectRay(x, y), (float4)(0.0f, -10, 0.0f, 0.0f), 10, 0, 1000)){
        color = (float4)(hitData1.normal.x, hitData1.normal.y, hitData1.normal.z, 1.0f);
    }
    struct HitData hitData2;
    if (checkIfHit(&hitData2, projectRay(x, y), (float4)(0.0f, 0, 0.0f, 0.0f), 1, 0, 1000)){
        color = (float4)(0.0f, 1.0f, 0.0f, 1.0f);
    }

    write_imagef(outputImage, (int2)(x, y), (float4)(sqrt(color.x), sqrt(color.y), sqrt(color.z), 1.0f));
}
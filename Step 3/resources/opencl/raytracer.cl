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

struct Triangle {
    float4 vertex1, vertex2, vertex3;
    float4 normal1, normal2, normal3;
    float4 uv1, uv2, uv3;
};

struct TriangleArray {
    unsigned long size;
    struct Triangle triangles[maxTriangleCount];
};

struct Object { 
    float4 min;
    float4 max;
    float4 position;
    struct TriangleArray triangleArray;
};

struct Ray {
    // the starting point for our ray
    float4 start;
    // and the direction it is currently traveling
    float4 direction;
    float4 inverseDirection;
};

float4 along(struct Ray ray, float length) { 
    return ray.start + length * ray.direction; 
}

float magnitude(float4 vec){
    return sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z + vec.w * vec.w);
}

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
    // the new ray to be cast if scattered
    struct Ray newRay;
    // the color of the material
    float4 attenuationColor;
};

// as close to a 1 to 1 copy from the cpu ray tracer
bool checkForTriangleIntersection(struct HitData* data, struct Triangle theTriangle, float4 position, struct Ray ray, float min, float max) {
    const float EPSILON = 0.0000001f;

    float4 edge1, edge2, s, h, q;
    float a, f, u, v;
    edge1 = (theTriangle.vertex2 + position) - (theTriangle.vertex1 + position);
    edge2 = (theTriangle.vertex3 + position) - (theTriangle.vertex1 + position);

    h = cross(ray.direction, edge2);
    a = dot(edge1, h);

    if (a > -EPSILON && a < EPSILON) {
        return false;
    }

    f = 1.0f / a;
    s = ray.start - (theTriangle.vertex1 + position);
    u = f * dot(s, h);

    if (u < 0.0f || u > 1.0f) {
        return false;
    }

    q = cross(s, edge1);
    v = f * dot(ray.direction, q);
    if (v < 0.0f || u + v > 1.0f) {
        return false;
    }

    // At this stage we can compute t to find out where the intersection point is on the line.
    float t = f * dot(edge2, q);

    // keep t in reasonable bounds, ensuring we respect depth
    if (t > EPSILON && t >= min && t <= max) {
        // ray intersects
        float4 rayIntersectionPoint = along(ray, t);
        float4 normal = theTriangle.normal1;
        
        // calculate triangle berry centric coords
        // first we need the vector that runs between the vertex and the intersection point for all three vertices
        // we must subtract the position of the triangle from the intersection point because this calc must happen in triangle space not world space.
        // you won't believe the time it took me to figure this out, since the U coord was correct but the V coord was always 1.
        float4 vertex1ToIntersect = theTriangle.vertex1 - (rayIntersectionPoint - position);
        float4 vertex2ToIntersect = theTriangle.vertex2 - (rayIntersectionPoint - position);
        float4 vertex3ToIntersect = theTriangle.vertex3 - (rayIntersectionPoint - position);
        
        // the magnitude of the cross product of two vectors is double the area formed by the triangle of their intersection.
        float4 fullAreaVec = cross(theTriangle.vertex1 - theTriangle.vertex2, theTriangle.vertex1 - theTriangle.vertex3);
        float4 areaVert1Vec = cross(vertex2ToIntersect, vertex3ToIntersect);
        float4 areaVert2Vec = cross(vertex3ToIntersect, vertex1ToIntersect);
        float4 areaVert3Vec = cross(vertex1ToIntersect, vertex2ToIntersect);
        float fullArea = 1.0 / magnitude(fullAreaVec);
        // scale the area of sub triangles to be proportion to the area of the triangle
        float areaVert1 = magnitude(areaVert1Vec) * fullArea;
        float areaVert2 = magnitude(areaVert2Vec) * fullArea;
        float areaVert3 = magnitude(areaVert3Vec) * fullArea;
        
        // since we are calculating UV coords, the hard interpolation part is already done.
        // so use said calculation to determine the overall normal based on the 3 individual vertexes
        normal = theTriangle.normal1 * areaVert1 + theTriangle.normal2 * areaVert2 + theTriangle.normal3 * areaVert3;
        
        // that area is how much each UV factors into the final UV coord
        // since the z and w component isn't used it's best to do this individually. (Where's that TODO on lower order vectors!!!)
        float t_u = theTriangle.uv1.x * areaVert1 + theTriangle.uv2.x * areaVert2 + theTriangle.uv3.x * areaVert3;
        float t_v = theTriangle.uv1.y * areaVert1 + theTriangle.uv2.y * areaVert2 + theTriangle.uv3.y * areaVert3;
        
        data->hitPoint = rayIntersectionPoint;
        data->normal = normal;
        data->length = t;
        data->u = t_u;
        data->v = t_v;
        return true;
    }
    
    return false;
}

// // // reads one unsigned long of data from the objects buffer.
// unsigned long getUnsignedLong(__global unsigned char* buffer, unsigned long* currentByte){
//      long val = *((global unsigned long*)(buffer));
//      *currentByte += sizeof(unsigned long);
//      return val;
// }

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

bool checkForWorldIntersection(struct HitData* data, __global struct Object* objects, struct Ray ray, float min, float max){
    data->length = max;
    bool hit = false;
    // brute-force check on all the objects in the world.
    for (int i = 0; i < objectCount; i++){
        struct TriangleArray triangleArray = objects[i].triangleArray;
        for (int j = 0; j < triangleArray.size; j++){
            if (checkForTriangleIntersection(data, triangleArray.triangles[j], objects[i].position, ray, min, data->length)){
                hit = true;
            }
        }
    }
    return hit;
}

bool scatter(struct ScatterResults* results, __global unsigned char* randoms, struct Ray ray, struct HitData data){
    const float EPSILON = 0.0000001f;
    int x = get_global_id(0);
    unsigned long cb = x * sizeof(float4);
    float4 newRay = data.normal + getVector(randoms, &cb);

    // rays that are close to zero are liable to floating point precision errors
    if (newRay.x < EPSILON && newRay.y < EPSILON && newRay.z < EPSILON && newRay.w < EPSILON)
        newRay = data.normal;

    struct Ray nRay;
    nRay.start = data.hitPoint;
    nRay.direction = newRay;
    nRay.inverseDirection = 1/nRay.direction;
    results->newRay = nRay;
    results->attenuationColor = (float4)(1.0f, 0.0f, 0.0f, 1.0f);
    return true;
}

float4 raycastI(__global unsigned char* randoms, __global struct Object* objects, struct Ray ray){
    struct Ray localRay = ray;
    float4 color = (float4)(1.0f, 1.0f, 1.0f, 1.0f);
    for (int i = 0; i < MAX_DEPTH; i++){
        struct HitData hit;
        if (checkForWorldIntersection(&hit, objects, ray, 0.001f, 100000.0f)){
            color = (float4)(1.0f, 0.0f, 0.0f, 1.0f);
        }
        /*if (hit.hit){
            struct ScatterResults results = scatter(randoms, localRay, hit);
            if (results.scattered){
                color = color * results.attenuationColor;
                localRay = results.newRay;
            } else {
                color = (float4)(0.0,0.0,0.0,0.0);
                break;
            }
        } else {
            // since we didn't hit, we hit the sky.
            color = color * (float4)(0.5, 0.7, 1.0, 1.0);
            // if we don't hit we cannot keep looping.
            break;
        }*/
    } 
    return color;
}

__kernel void raycast(__write_only image2d_t outputImage, __global struct Object* objects, __global unsigned char* randoms) {
    unsigned long currentByte = 0;

    int x = get_global_id(0);
    unsigned long cb = x * sizeof(float4);
    float4 randomVector = getVector(randoms, &cb);
    int y = get_global_id(1);

    float4 color = (float4)(0.0);
    //for (int i = 0; i < MAX_PER_PIXEL; i++){
        color = color + raycastI(randoms, objects, projectRay(x + randomVector.x, y + randomVector.z));
    //}
    float scaleFactor = 1.0 / MAX_PER_PIXEL;
    write_imagef(outputImage, (int2)(x, y), (float4)(sqrt(color.x * scaleFactor), sqrt(color.y * scaleFactor), sqrt(color.z * scaleFactor), 1.0f));
    //write_imagef(outputImage, (int2)(x, y), (float4)(color.x, color.y, color.z, 100000.0f));
}
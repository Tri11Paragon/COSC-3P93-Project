__constant sampler_t sampler = CLK_NORMALIZED_COORDS_TRUE |
                               CLK_ADDRESS_CLAMP_TO_EDGE |
                               CLK_FILTER_NEAREST;


float getColor(int x, int y){
    return sin((x / 512.0) / cos(y / 512.0)) * cos((x / 512.0) * sin(y / 512.0));
}

float getColorInv(int x, int y){
    return sin((x / 256.0) / cos(y / 128.0)) * cos((x / 512.0) * sin(y / 512.0));
}

float getColorCRR(int x, int y){
    return x / y * sin(y / x * 512.0) - cos(y / 128.0);
}

float finalFactor(float x){
    return cos(x / 32.0) * 32.0;
}

float factor(int x, int y){
    return sin(x / 32.0) * finalFactor(y);
}

float oldFactor(int x){
    return sin(x / 128.0) / 2;
}

__kernel void drawImage(__write_only image2d_t output, __read_only image2d_t input) {
    
    // Get the index of the current element to be processed
    int x = get_global_id(0);
    int y = get_global_id(1);

    float4 oldImage = read_imagef(input, (int2)(x, y));

    float r = oldImage.x * oldFactor(x + y);
    float g = oldImage.y * oldFactor(x + y);
    float b = oldImage.z * oldFactor(x + y);

    float colorR = (getColor(x, y) * r) / 32.0;
    float colorG = (getColorInv(x, y) * g) / 32.0;
    float colorB = (getColorCRR(x, y) * b) / 32.0;

    float4 totalColor = (float4)(colorR, colorG, colorB, 1.0);
    for (int i = -3; i <= 3; i++){
        for (int j = -3; j <= 3; j++){
            totalColor = totalColor + (read_imagef(input, (int2)(x + i, y + j)) * factor(i, j));
        }
    }
    totalColor = totalColor / (float4)(9.0, 9.0, 9.0, 1.0);

    write_imagef(output, (int2)(x, y), (float4)(totalColor.x, totalColor.y, totalColor.z, 1.0));
}

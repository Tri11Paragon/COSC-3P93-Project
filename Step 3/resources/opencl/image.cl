
__kernel void drawImage(__write_only image2d_t output) {
    
    // Get the index of the current element to be processed
    int i = get_global_id(0);
    int j = get_global_id(1);

    float color = sin(i / 10.0) * sin (j / 10.0);

    write_imagef(output, (int2)(i, j), (float4)(color, color, color, 1.0));
}

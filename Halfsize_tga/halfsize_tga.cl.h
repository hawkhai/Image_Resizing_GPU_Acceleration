
#define __kernel
#define __global

typedef unsigned char uchar;

size_t get_global_id(int index) {
    return 0;
}
size_t get_global_size(int index) {
    return 0;
}

__kernel void gpuip_scale(
    uchar imgPixelSize,
    uchar numChannel,
    __global uchar* imgInBuffer,
    __global uchar* imgOpBuffer) {

    size_t i = get_global_id(0); // The i'th pixel(index) of output image
    size_t j = get_global_id(1); // The j'th pixel(index) of output image
    size_t countX = get_global_size(0); // Width of image
    size_t countY = get_global_size(1); // Height of image

    size_t index00 = 0, index01 = 0, index10 = 0, index11 = 0;

    if (numChannel == 1)
    {
        size_t index = (j * countX) + i;

        index00 = ((j * 2) * countX) + (i * 2);
        index01 = ((j * 2) * countX) + (i * 2) + 1;
        index10 = (((j * 2) + 1) * countX) + (i * 2);
        index11 = (((j * 2) + 1) * countX) + (i * 2) + 1;

        imgOpBuffer[index] = (
            (size_t)imgInBuffer[index00] +
            (size_t)imgInBuffer[index01] +
            (size_t)imgInBuffer[index10] +
            (size_t)imgInBuffer[index11]) / 4;
    }

    if (numChannel == 3 && imgPixelSize == 24)
    {
        size_t index = (j * countX * 3) + (i * 3);

        i = i * 6;

        size_t k = 0;
        for (k = 0; k < 3; k++) {

            index00 = (j * countX * 3) + i + k;
            index01 = (j * countX * 3) + (i + 3 + k);
            index10 = ((j + 1) * countX * 3) + i + k;
            index11 = ((j + 1) * countX * 3) + (i + 3 + k);

            imgOpBuffer[index + k] = (
                (size_t)imgInBuffer[index00] +
                (size_t)imgInBuffer[index01] +
                (size_t)imgInBuffer[index10] +
                (size_t)imgInBuffer[index11]) / 4;
        }
    }

    if (numChannel == 4)
    {
        size_t index = (j * countX * 4) + (i * 4);

        i = i * 8;

        size_t k = 0;
        for (k = 0; k < 4; k++) {

            index00 = (j * countX * 4) + i + k;
            index01 = (j * countX * 4) + (i + 4 + k);
            index10 = ((j + 1) * countX * 4) + i + k;
            index11 = ((j + 1) * countX * 4) + (i + 4 + k);

            imgOpBuffer[index + k] = (
                (size_t)imgInBuffer[index00] +
                (size_t)imgInBuffer[index01] +
                (size_t)imgInBuffer[index10] +
                (size_t)imgInBuffer[index11]) / 4;
        }
    }
}
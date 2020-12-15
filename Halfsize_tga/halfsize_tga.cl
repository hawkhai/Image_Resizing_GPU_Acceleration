/*
 *------------------------------------------------------------------------------
 * File : halfsize_tga.cpp
 *------------------------------------------------------------------------------
 *
 * Copyright 2020 
 * Author - Mohammed Muddasser
 *------------------------------------------------------------------------------
 * Permission to use, copy, modify, distribute and sell this software and its 
 * documentation for any purpose is hereby granted without fee, provided that 
 * the above copyright notice appear in all copies and that both that copyright 
 * notice and this permission notice appear in supporting documentation. 
 * Binaries may be compiled with this software without any royalties or 
 * restrictions. 
*/

/*
 * halfSizeTGAKernel :
 * Description - Compute the half sized image.
*/
__kernel void halfSizeTGAKernel (uchar imgPixelSize, uchar noChannel, __global uchar* imgInBuffer, __global uchar* imgOpBuffer) {

    size_t i = get_global_id(0);                                                                                    // The i'th pixel(index) of output image
	size_t j = get_global_id(1);                                                                                    // The j'th pixel(index of output image
    size_t countX = get_global_size(0);                                                                             // Width of image
    size_t countY = get_global_size(1);                                                                             // Height of image

	size_t index00 = 0, index01 = 0, index10 = 0, index11 = 0;

    //Number of channels can be 1, 3 or 4 only, taken care in host
    if (noChannel == 1)							// 8 bit Black and white image
    {
		// Index pointing to the corresponding position in the input buffer
		size_t index = (j * countX) + i;

		index00 = ((j * 2) * countX) + (i*2);
		index01 = ((j * 2) * countX) + (i*2) + 1;
		index10 = (((j * 2 )+ 1) * countX) + (i*2);
		index11 = (((j * 2) + 1) * countX) + (i*2) + 1;

		// Average the surrounding 4 pixel values to shrink to 1 pixel
        imgOpBuffer[index] = ((size_t)imgInBuffer[index00] + (size_t)imgInBuffer[index01] + (size_t)imgInBuffer[index10] + (size_t)imgInBuffer[index11]) / 4;
    }
    
    if (noChannel == 3)
    {
		if (imgPixelSize == 16)
		{
			// To be implemented
		}

		if(imgPixelSize == 24)					// 24 bit RGB
		{
			// Adjust the index ponting in the input buffer, so as to jump 3 bytes every workitem 
			size_t index = (j * countX * 3) + (i * 3);

            // Adjust the index pointing to the input data array by jumping 8 byte positons for every new work item
			i = i * 6;

			// Process Blue pixel
			index00 = (j * countX * 3) + i;
			index01 = (j * countX * 3) + (i + 3);
			index10 = ((j + 1) * countX * 3) + i;
			index11 = ((j + 1) * countX * 3) + (i + 3);

			// Average the Blue value from the surrounding 4 pixel values to shrink to 1 pixel 
			imgOpBuffer[index] = ((size_t)imgInBuffer[index00] + (size_t)imgInBuffer[index01] + (size_t)imgInBuffer[index10] + (size_t)imgInBuffer[index11]) / 4;

			// Process Green pixel
			index00 = (j * countX * 3) + (i + 1);
			index01 = (j * countX * 3) + (i + 4);
			index10 = ((j + 1) * countX * 3) + (i + 1);
			index11 = ((j + 1) * countX * 3) + (i + 4);

			// Average the Green value from the surrounding 4 pixel values to shrink to 1 pixel 
			imgOpBuffer[index+1] = ((size_t)imgInBuffer[index00] + (size_t)imgInBuffer[index01] + (size_t)imgInBuffer[index10] + (size_t)imgInBuffer[index11]) / 4;

			// Process Red pixel
			index00 = (j * countX * 3) + (i + 2);
			index01 = (j * countX * 3) + (i + 5);
			index10 = ((j + 1) * countX * 3) + (i + 2);
			index11 = ((j + 1) * countX * 3) + (i + 5);

			// Average the Red value from the surrounding 4 pixel values to shrink to 1 pixel 
			imgOpBuffer[index+2] = ((size_t)imgInBuffer[index00] + (size_t)imgInBuffer[index01] + (size_t)imgInBuffer[index10] + (size_t)imgInBuffer[index11]) / 4;
 
		}
    }

	if(noChannel == 4)							// 32 bit RGBA - Calculate all R, G, B and A values serially
	{
		// Adjust the index ponting in the input buffer, so as to jump 4 bytes every workitem 
		size_t index = (j * countX * 4) + (i * 4);

		// Adjust the index pointing to the input data array by jumping 8 byte positons for every new work item
		i = i * 8;

		// Process Blue pixel
        index00 = (j * countX * 4) + i;
        index01 = (j * countX * 4) + (i + 4);
        index10 = ((j + 1) * countX * 4) + i;
        index11 = ((j + 1) * countX * 4) + (i + 4);

		// Average the Blue value from the surrounding 4 pixel values to shrink to 1 pixel 
		imgOpBuffer[index] = ((size_t)imgInBuffer[index00] + (size_t)imgInBuffer[index01] + (size_t)imgInBuffer[index10] + (size_t)imgInBuffer[index11]) / 4;

		// Process Green pixel
        index00 = (j * countX * 4) + (i + 1);
        index01 = (j * countX * 4) + (i + 5);
        index10 = ((j + 1) * countX * 4) + (i + 1);
        index11 = ((j + 1) * countX * 4) + (i + 5);

		// Average the Green value from the surrounding 4 pixel values to shrink to 1 pixel 
		imgOpBuffer[index+1] = ((size_t)imgInBuffer[index00] + (size_t)imgInBuffer[index01] + (size_t)imgInBuffer[index10] + (size_t)imgInBuffer[index11]) / 4;

		// Process Red pixel
        index00 = (j * countX * 4) + (i + 2);
        index01 = (j * countX * 4) + (i + 6);
        index10 = ((j + 1) * countX * 4) + (i + 2);
        index11 = ((j + 1) * countX * 4) + (i + 6);

		// Average the Red value from the surrounding 4 pixel values to shrink to 1 pixel 
		imgOpBuffer[index+2] = ((size_t)imgInBuffer[index00] + (size_t)imgInBuffer[index01] + (size_t)imgInBuffer[index10] + (size_t)imgInBuffer[index11]) / 4;

		// Process Alpha pixel
        index00 = (j * countX * 4) + (i + 3);
        index01 = (j * countX * 4) + (i + 7);
        index10 = ((j + 1) * countX * 4) + (i + 3);
        index11 = ((j + 1) * countX * 4) + (i + 7);

		// Average the Alpha value from the surrounding 4 pixel values to shrink to 1 pixel 
		imgOpBuffer[index+3] = ((size_t)imgInBuffer[index00] + (size_t)imgInBuffer[index01] + (size_t)imgInBuffer[index10] + (size_t)imgInBuffer[index11]) / 4;
	}
}
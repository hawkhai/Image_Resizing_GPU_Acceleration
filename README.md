# GPU based acceleration for TGA image resizing.

## Author: Mohammed Muddasser

## Objective
The submitted is an OpenCL tool which loads a .tga image file and resizes it to half of its original width and height, and then saves it back to disk, as a new .tga file. A zipped Visual Studio project is delivered.

## Solution Overview:
The supports 8 bit Monochrome, 24-bit uncompressed RGB and 32-bit uncompressed RGBA. The RLE support can be plugged in, where additional implementation involves reading the decoded input image data into the host buffer. Hence an extension can be provided with the current code. The kernel extension to support the 16 bit RGB and MAP has be to be implemented as one of the if/else cases in the kernel code. The code has been designed to be modular so easy extension and plugin is possible.

The code is distributed in 3 sections with 5 files:

  1. The Host execution- halfsize_tga.cpp, halfsize_tga.h
  2. The TGA helper code- tga_helper.cpp, tga_helper.h
  3. The kernel execution - halfsize_tga.cl

The TGA image is read, parsed and stored in the structure TGA. By default, all the elements are public such that the member elements can be accessed directly. This structure can be replaced by a class, so as to make all member variables private and provide a Get() and Set() function for each member variable respectively, to provide data hiding and protection.
```C++
  // TGA Image as structure
  struct TGA {

    // By default all kept public
    // Member Variables
    tgaHeader				      mHeader;						  // Header of the image decoded
    uint32_t				      mPixelDataLen;			  // Pixel data length
    std::vector<uint8_t>	mID;							    // ID 
    std::vector<uint8_t>	mColorMapSpec;				// ColorMapSpec
    std::vector<uint8_t>	mPixelData;						// Pixel Data

    // Default Constructor
    TGA() {
      mHeader = {0};
      mPixelDataLen = 0;
    }

    // Parameterised Constructor
      TGA(const char *imgName) {
          read(imgName);
    }

    // Member functions
    void read(const char *imgName);
    bool write(const char *imgName);
  };
```

## Kernel Implementation:
• The half sizing is done by clustering 4 pixels (as a box) and averaging the pixel values.
• The buffers and image data is always represented using 1 byte array.
• The number of workitems are always equal to the size of the output image, i.e (InImageWidth/2 * InImageHeight/2). For the 24-bit RGB and 32-bit RGBA a serial computation of all R, G, B and A values is done in one execution thread and core.

## Delivery:
A zipped Visual Studio solution directory is delivered, which can be unzipped, to review the code, and built in Debug and Release mode. The desired output can be seen on the console.



#include <CL\cl.hpp>
#include "tga_helper.h"

using namespace std;

//=======================================================================
//  
// GLOBAL VARIABLES
//
//=======================================================================
cl::Program program;
cl::Context context;
cl::CommandQueue que;
cl::Device device;
std::vector<cl::Platform> platforms;

int main()
{
	// cl::Platform platforms;

	//============================================================
	// Initialization of the GPU
	//============================================================

	cl::Platform::get(&platforms);
	if (platforms.size() == 0) {
		std::cerr << "No platforms found" << std::endl;
		return 1;
	}

	int platformId = 0;
	size_t i;
	std::cout << "Finding Platforms\n";
	for (i = 0; i < platforms.size(); i++) {
		/*if (platforms[i].getInfo<CL_PLATFORM_NAME>() == "AMD Accelerated Parallel Processing") {
			platformId = i;
			break;
		}*/
		platformId = i;
		std::cout << platforms[i].getInfo<CL_PLATFORM_NAME>() << "\n";
	}

	std::cout << platformId << "\n";
	cl_context_properties prop[4] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platforms[platformId](), 0, 0 };
	std::cout << "Using platform '" << platforms[platformId].getInfo<CL_PLATFORM_NAME>() << "' from '" << platforms[platformId].getInfo<CL_PLATFORM_VENDOR>() << "'" << std::endl;

	// READ
	// Read a TGA file
	char * fileNameRd = "../images/utc16.tga";
	TGA imgRead(fileNameRd);












	// WRITE 
	// Copy data from device buffer and write to new image file
	TGA imgWrite;
	imgWrite.mHeader		= imgRead.mHeader;
	imgWrite.mPixelDataLen	= imgRead.mPixelDataLen;

	// Update the ID
	copy(imgRead.mID.begin(), imgRead.mID.end(), back_inserter(imgWrite.mID));

	// Update the ColorMapSpec 
	copy(imgRead.mColorMapSpec.begin(), imgRead.mColorMapSpec.end(), back_inserter(imgWrite.mColorMapSpec));

	// Update the PixelData
	//imgWrite.mPixelData.resize(imgWrite.mPixelDataLen);
	// Copy from device to host
	//TODO
	copy(imgRead.mPixelData.begin(), imgRead.mPixelData.end(), back_inserter(imgWrite.mPixelData));

	char* fileNameWr = "../images/test_utc16.tga";
	if (imgWrite.write(fileNameWr))
	{
		fprintf(stderr, "Error: Failed to write the new image. \n");
		throw "Error: Failed to write the new image. \n";
	}

	char * fileName2 = "../images/test_utc16.tga";
	TGA img2(fileName2);
}


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

//unsigned char* readTGA(char* filename)
//{
//
//	FILE* img = fopen(filename, "rb");
//	unsigned char info[54];
//	fread(info, sizeof(unsigned char), 54, img); // read the 54-byte header
//
//											   // extract image height and width from header
//	int width = *(int*)&info[18];
//	int height = *(int*)&info[22];
//
//	int size = 3 * width * height;
//	unsigned char* data = new unsigned char[size]; // allocate 3 bytes per pixel
//	fread(data, sizeof(unsigned char), size, img); // read the rest of the data at once
//	fclose(img);
//
//	for (int i = 0; i < size; i += 3)
//	{
//		unsigned char tmp = data[i];
//		data[i] = data[i + 2];
//		data[i + 2] = tmp;
//	}
//
//	return data;
//}


int main()
{
	// cl::Platform platforms;

	//===========================================================
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

	/* Read a TGA file */
	TGA Img1("CTC32.TGA");

}


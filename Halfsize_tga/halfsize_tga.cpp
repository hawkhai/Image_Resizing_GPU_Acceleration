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

	context = cl::Context(CL_DEVICE_TYPE_GPU, prop);
	//	Get a device of the context
	int deviceNr = argc < 2 ? 1 : atoi(argv[1]);
	std::cout << "Using device " << deviceNr << " / " << context.getInfo<CL_CONTEXT_DEVICES>().size() << std::endl;
	ASSERT(deviceNr > 0);
	ASSERT((size_t)deviceNr <= context.getInfo<CL_CONTEXT_DEVICES>().size());
	device = context.getInfo<CL_CONTEXT_DEVICES>()[deviceNr - 1];
	std::vector<cl::Device> devices;
	devices.push_back(device);
	OpenCL::printDeviceInfo(std::cout, device);

	//	Create a command que
	que = cl::CommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE);

	// READ
	// Read a TGA file
	char * fileNameRd = "../images/utc16.tga";
	TGA imgRead(fileNameRd);

	// Number of channels
	uint8_t noChannel = imgRead.mHeader.imgPixelSize / 8;				// can be 1, 3 or 4
	if (noChannel == 2) noChannel = 3;									// 16-bit is 3 channel with R5G6B5.

	// WRITE 
	// Copy data from device buffer and write to new image file
	TGA imgWrite;
	imgWrite.mHeader = imgRead.mHeader;
	imgWrite.mPixelDataLen = imgRead.mPixelDataLen;

	// Update the ID
	copy(imgRead.mID.begin(), imgRead.mID.end(), back_inserter(imgWrite.mID));

	// Update the ColorMapSpec 
	copy(imgRead.mColorMapSpec.begin(), imgRead.mColorMapSpec.end(), back_inserter(imgWrite.mColorMapSpec));

	// GPU 
	// Parameters to compute total buffer size to be allocated to V, C and R buffers
	size_t wgSize = 16;                                       // Work Group size

	// For Timimg Analysis
	cl::Event event_kernel1;
	cl::Event event_write1;
	cl::Event event_write2;
	cl::Event event_read1;

	// Create host memory for initialising output buffer
	std::vector<cl_char>  imgOpBufferDefVal(imgWrite.mPixelDataLen * sizeof(cl_char));

	// Initilaization of host memory output buffer
	memset(imgOpBufferDefVal.data(), 0, (imgWrite.mPixelDataLen * sizeof(cl_char)));

	// Allocate space (buffers) for output data on the device 
	// Input buffer for pixel data
	cl::Buffer imgInBuffer(context, CL_MEM_READ_ONLY, imgRead.mPixelDataLen * sizeof(cl_char));		
	// Output buffer for pixel data
	cl::Buffer imgOpBuffer(context, CL_MEM_READ_WRITE, ((imgWrite.mPixelDataLen * sizeof(cl_char)));

	// Copy from host to device memory
	// Copy image data to Buffer
	que.enqueueWriteBuffer(imgInBuffer, true, 0, imgRead.mPixelDataLen * sizeof(uint8_t), imgRead.mPixelData.data(), NULL, &event_write1);
	// Copy the output data with zeros, init value
	que.enqueueWriteBuffer(imgOpBuffer, true, 0, imgWrite.mPixelDataLen * sizeof(uint8_t), imgOpBufferDefVal.data(), NULL, &event_write2);

	// Wait until all write events complete
	event_write1.wait();

	// New kernel object for computation
	cl::Kernel halfSizeTGAKernel(program, "halfSizeTGAKernel");

	// Kernel1 arguments
	halfSizeTGAKernel.setArg<cl_char>(0, (cl_char)imgRead.mHeader.imgPixelSize);
	halfSizeTGAKernel.setArg<cl_char>(1, (cl_char)noChannel);
	halfSizeTGAKernel.setArg<cl::Buffer>(2, imgInBuffer);
	halfSizeTGAKernel.setArg<cl::Buffer>(3, imgOpBuffer);

	// Launch the kernel1
	que.enqueueNDRangeKernel(halfSizeTGAKernel, 0, cl::NDRange((size_t)imgRead.mHeader.imgWidth, (size_t)imgRead.mHeader.imgHeight), cl::NDRange(wgSize * wgSize), NULL, &event_kernel1);

	// Wait until kernel1 is complete
	event_kernel1.wait();

	// Copy output data back to host
	que.enqueueReadBuffer(imgOpBuffer, true, 0, ((imgWrite.mPixelDataLen * sizeof(cl_char))), imgWrite.mPixelData.data(), NULL, &event_read1);

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

	// TIMING ANALYSIS START 

#if TGA_DEBUG
	// Timing Analysis
	Core::TimeSpan time1    = OpenCL::getElapsedTime(event_kernel1);
	Core::TimeSpan time2    = OpenCL::getElapsedTime(event_write1);
	Core::TimeSpan time3    = OpenCL::getElapsedTime(event_write2);
	Core::TimeSpan time5    = OpenCL::getElapsedTime(event_kernel1);
	Core::TimeSpan time6    = OpenCL::getElapsedTime(event_read1);

	std::cout << std::endl << std::endl;
	std::cout <<"Kernel time for Row Pointer computation                : " << time1 << std::endl;
	std::cout <<"Write and que time for Values Buffer                   : " << time2 << std::endl;
	std::cout <<"Write and que time for Columns Buffer                  : " << time3 << std::endl;
	std::cout <<"Write and que time for RowPointers Buffer              : " << time4 << std::endl;
	std::cout <<"Kernel time for Val and Columns computation            : " << time5 << std::endl;
	std::cout <<"Copy time from kernel to compressed matrix Values      : " << time6 << std::endl;
	std::cout <<"Copy time from kernel to compressed matrix Columns     : " << time7 << std::endl;
	std::cout <<"Copy time from kernel to compressed matrix RowPointers : " << time8 << std::endl;

	Core::TimeSpan timeGPUD = time1+time2+time3+time4+time5+time6+time7+time8;

	std::cout <<"---------------------------------------------------------------------------------" << std::endl;
	std::cout <<"Total Time for computation of D matrix on the GPU      : "<< timeGPUD << std::endl;
	std::cout <<"---------------------------------------------------------------------------------" << std::endl << std::endl << std::endl;
#endif
}


//------------------------------------------------------------------------------
// File : halfsize_tga.cpp
//------------------------------------------------------------------------------

// Copyright 2020 
// Author - Mohammed Muddasser
//------------------------------------------------------------------------------
// Permission to use, copy, modify, distribute and sell this software and its 
// documentation for any purpose is hereby granted without fee, provided that 
// the above copyright notice appear in all copies and that both that copyright 
// notice and this permission notice appear in supporting documentation. 
// Binaries may be compiled with this software without any royalties or 
// restrictions. 
//

#include "halfsize_tga.h"

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

//------------------------------------------------------------------------------------------------
// The main function.
//
// int main(int argc, char *argv[])
// Input Parameters :	argc	- number of srguments passed
//						*argv[] - Arguments passed		
// Return Parameters:	0 - no error
//						other - error
// 
//-------------------------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
	// Read the filenames passed from command line
	char * InpFile = argv[1];
	char * OpFile  = argv[2];

	fprintf(stdout, "\n-------------------------------------------------------------\n");
	fprintf(stdout, "                     Running Halfsize_TGA						\n");

	//============================================================
	// Initialization of the GPU
	//============================================================
	std::vector<cl::Platform> platforms;				// Vector to get all platforms
	std::vector<cl::Device> devices_available;
	std::vector<cl::Device> devices;
	uint8_t platformId = 0;								// Platform ID

#if TGA_DEBUG
	fprintf(stdout, "\n-------------------------------------------------------------\n");
	fprintf(stdout, "Finding Platforms and OpenCL supported GPU devices:\n");
#endif

	// Get the platforms
	cl::Platform::get(&platforms);
	if (platforms.size() == 0) {
		fprintf(stderr, "Error: No OpenCL supported platforms available!\n");
		return -1;
	}

	// Check if the platform and devices exist
	bool platformflg = 0, deviceflg	= 0;								
	for (uint8_t i = 0; i<(uint8_t)platforms.size(); i++) {
		// clear the vector
		devices_available.clear();

		// If no platform defined
		if (TGA_PLATFORM_DEFAULT)
		{
			platformflg = 1;

			// Fetch all OpenCL supported GPU devices in the platform
			platforms[i].getDevices(CL_DEVICE_TYPE_GPU, &devices_available);

			// If no devices found in platform
			if (devices_available.size() == 0)
			{
				//fprintf(stderr, "No OpenCL supported devices found in platform %s.\n", platforms[i].getInfo<CL_PLATFORM_NAME>());
				cout << "No OpenCL supported devices found in platform " << platforms[i].getInfo<CL_PLATFORM_NAME>() << " \n";
				continue;										// no device found in plattform i
			}

			// For now take the first GPU Device. Extension can be done to take a particular GPU if multiple present.
			devices.push_back(devices_available[0]);
			platformId = i;
			deviceflg = 1;
			break;
		}

		// If specific platform defined
		else
		{
			if (platforms[i].getInfo<CL_PLATFORM_NAME>() == TGA_PLATFORM_NAME)
			{
				platformflg = 1;
				// Fetch all OpenCL supported GPU devices in the platform
				platforms[i].getDevices(CL_DEVICE_TYPE_CPU, &devices_available);

				// If no devices found in platform
				if (devices_available.size() == 0)
				{
					//fprintf(stderr, "No OpenCL supported devices found in platform %s.\n", platforms[i].getInfo<CL_PLATFORM_NAME>()); to prevent warning
					cout << "No OpenCL supported devices found in platform " << platforms[i].getInfo<CL_PLATFORM_NAME>() << " \n";
					break;												// no device found in platform
				}

				// If device number 
				if (TGA_DEVICE_NUMBER > devices_available.size() || TGA_DEVICE_NUMBER <= 0)
				{
					fprintf(stderr, "Error: Invalid device number %d", TGA_DEVICE_NUMBER);
					break;												// no device found in platform
				}

				// TGA_DEVICE_NUMBER device taken (Refer macro in tga_helper.h)
				devices.push_back(devices_available[TGA_DEVICE_NUMBER]);
				platformId = i;
				break;
			}
		}
	}

	// If no devices found in the system
	if ((bool)TGA_PLATFORM_DEFAULT & !deviceflg)
	{
		fprintf(stderr, "No OpenCL supported devices found in any platform.\n");
		return -1;														
	}

	if (!platformflg)
	{
		fprintf(stderr, "The Platform %s not found.\n", TGA_PLATFORM_NAME);
		return -1;
	}

	// If no devices found in the system
	if ((bool)!(TGA_PLATFORM_DEFAULT) & !deviceflg)
	{
		fprintf(stderr, "No OpenCL supported devices found in platform %s.\n", TGA_PLATFORM_NAME);
		return -1;
	}

	// Set device and create Context
	auto device = devices.back();		
	cl::Context context(device);

#if TGA_DEBUG
	// fprintf not used to prevent warning 
	std::cout << "Using platform '" << platforms[platformId].getInfo<CL_PLATFORM_NAME>() << "' from '" 
									<< platforms[platformId].getInfo<CL_PLATFORM_VENDOR>() << "'" << std::endl;
	std::cout << "Using device type '" << device.getInfo<CL_DEVICE_NAME>() << "' from vendor '" 
										<< device.getInfo<CL_DEVICE_VENDOR>() << "'" << std::endl;
#endif

	// Create iftsream object and sources to load the respective file
	std::ifstream halfsize_tga("halfsize_tga.cl");
	std::string src(std::istreambuf_iterator<char>(halfsize_tga), (std::istreambuf_iterator<char>()));
	cl::Program::Sources sources(1, std::make_pair(src.c_str(), src.length() + 1));

	// Load the source code
	cl::Program program(context, sources);

	// Compile the source code. Compiled for devices supporitng OpenCL version 1.3 and above.
	auto buildErr = program.build("-cl-std=CL1.2");

	if (buildErr != CL_SUCCESS)
	{
		fprintf(stderr, "Error %d while building the kernel.\n", buildErr);

		// Print error log for kernel code
		if (buildErr == CL_BUILD_PROGRAM_FAILURE) {
			// Determine the size of the log
			size_t log_size;
			clGetProgramBuildInfo(program(), device(), CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

			// Allocate memory for the log
			char *log = (char *)malloc(log_size);

			// Get the log
			clGetProgramBuildInfo(program(), device(), CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

			// Print the log
			fprintf(stderr, "%s\n", log);
		}
		return -1;
	}

	//	Create a command que
	que = cl::CommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE);

	//============================================================
	// BEGIN IMAGE PROCESSING
	//============================================================

	// PROCESSING TO READ IMAGE 
	// Read a TGA file
	fprintf(stdout, "\n-------------------------------------------------------------\n");
	fprintf(stdout, "\n");
	fprintf(stdout, "Reading \"%s\"... \n", InpFile);
	TGA imgRead(InpFile);
	fprintf(stdout, "Done. \n");

	fprintf(stdout, "Original size : %dX%d \n", imgRead.mHeader.imgWidth, imgRead.mHeader.imgHeight);
	
	// Number of channels
	uint8_t noChannel = imgRead.mHeader.imgPixelSize / 8;				// can be 1, 3 or 4
	if (noChannel == 2) noChannel = 3;									// 16-bit is 3 channel with R5G6B5.
	
	if(noChannel > 4 || noChannel < 1)
	{
		fprintf(stderr, "Error: Invalid number of channels i.e invalid pixel size field in header. %d.\n", noChannel);
		return -1;
	}

	// PROCESSING TO WRITE IMAGE 
	// Copy data from device buffer and write to new image file
	TGA imgWrite;

	// Set all header fields of the output image same as that of the input image
	imgWrite.mHeader			= imgRead.mHeader;	

	// Set image width of the output image is half the original
	imgWrite.mHeader.imgWidth	= imgRead.mHeader.imgWidth / 2;	

	// Set image height of the output image is half the original
	imgWrite.mHeader.imgHeight	= imgRead.mHeader.imgHeight / 2;	

	// Calculate the image data length of of the output image 
	imgWrite.mPixelDataLen		= imgWrite.mHeader.imgWidth * imgWrite.mHeader.imgHeight * (imgWrite.mHeader.imgPixelSize / 8);
	
	// Update the ID in output image sam as from the input image
	copy(imgRead.mID.begin(), imgRead.mID.end(), back_inserter(imgWrite.mID));

	// Update the ColorMapSpec data if present in output image same as from the input image (Only for extension of Map type images)
	copy(imgRead.mColorMapSpec.begin(), imgRead.mColorMapSpec.end(), back_inserter(imgWrite.mColorMapSpec));

	// Print 
	fprintf(stdout, "Resizing to: %dX%d \n", imgWrite.mHeader.imgWidth, imgWrite.mHeader.imgHeight);

	// GPU PROCESSING BEGIN
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
	cl::Buffer imgOpBuffer(context, CL_MEM_READ_WRITE, imgWrite.mPixelDataLen * sizeof(cl_char));

	// Copy from host to device memory
	// Copy image data to Buffer
	uint32_t errWrite1 = que.enqueueWriteBuffer(	imgInBuffer, true, 0, imgRead.mPixelDataLen * sizeof(uint8_t), 
													imgRead.mPixelData.data(), NULL, &event_write1);
	if (errWrite1 != CL_SUCCESS)
	{
		fprintf(stderr, "Error: Read from kernel failedwith error %d \n", errWrite1);
		//throw "Error: Read from kernel failedwith error %d \n", errWrite1;
	}

	// Wait until write2 event is complete
	event_write1.wait();

	// Copy the output data with zeros, init value
	uint32_t errWrite2 = que.enqueueWriteBuffer(	imgOpBuffer, true, 0, 
													imgWrite.mPixelDataLen * sizeof(uint8_t), imgOpBufferDefVal.data(), 
													NULL, &event_write2);
	if (errWrite2 != CL_SUCCESS)
	{
		fprintf(stderr, "Error: Read from kernel failedwith error %d \n", errWrite2);
		//throw "Error: Read from kernel failedwith error %d \n", errWrite2;
	}

	// Wait until write1 event is complete
	event_write2.wait();

	// New kernel object for computation
	cl::Kernel halfSizeTGAKernel(program, "halfSizeTGAKernel");

	// Kernel1 arguments
	halfSizeTGAKernel.setArg<cl_char>(0, (cl_char)imgRead.mHeader.imgPixelSize);
	halfSizeTGAKernel.setArg<cl_char>(1, (cl_char)noChannel);
	halfSizeTGAKernel.setArg<cl::Buffer>(2, imgInBuffer);
	halfSizeTGAKernel.setArg<cl::Buffer>(3, imgOpBuffer);

	// Parameters to compute total buffer size to be allocated to V, C and R buffers
	// Depends on image size. Needs to be smaller then the number of workitmes.
	size_t wgSize = 16;                                       // Work Group size

	// Launch the kernel1
	uint32_t errKernel = que.enqueueNDRangeKernel(	halfSizeTGAKernel, 0, 
													cl::NDRange((size_t)imgWrite.mHeader.imgWidth, (size_t)imgWrite.mHeader.imgHeight), 
													cl::NDRange(wgSize, wgSize), NULL, &event_kernel1);
	if (errKernel != CL_SUCCESS)
	{
		fprintf(stderr, "Error: Kernel run failed with error %d \n", errKernel);
		//throw "Error: Kernel run failed with error %d \n", errKernel;
	}

	// Wait until kernel1 is complete
	event_kernel1.wait();

	// Resize the image buffer
	imgWrite.mPixelData.resize(imgWrite.mPixelDataLen);

	// Copy output data back to host
	uint32_t errRead = que.enqueueReadBuffer(	imgOpBuffer, true, 0, 
												((imgWrite.mPixelDataLen * sizeof(cl_uchar))), imgWrite.mPixelData.data(), 
												NULL, &event_read1);
	if (errRead != CL_SUCCESS)
	{
		fprintf(stderr, "Error: Read from kernel failed with error %d \n", errRead);
		//throw "Error: Read from kernel failed with error %d \n", errRead);
	}

	// Print
	fprintf(stdout, "Done. \n");

	// Write to the file
	fprintf(stdout, "Saving \"%s\"... \n", InpFile);
	if (imgWrite.write(OpFile))
	{
		fprintf(stderr, "Error: Failed to write the new image. \n");
		throw "Error: Failed to write the new image. \n";
	}
	fprintf(stdout, "Done. \n");

#if TGA_DEBUG
	// Enable if - To read and test the written file
	fprintf(stderr, "Testing the new image. \n");
	TGA img2(OpFile);
#endif

	// GPU PROCESSING END
	//********************************************************************

	//********************************************************************
	// TIMING ANALYSIS 

	fprintf(stdout, "\n-------------------------------------------------------------\n");
	fprintf(stdout, "                        TIMING ANALYSIS						\n");
	fprintf(stdout, "\n");

	double timeW1 = getElapsedTimeMilliSec(event_write1);
	double timeW2 = getElapsedTimeMilliSec(event_write2);
	double timeK1 = getElapsedTimeMilliSec(event_kernel1);
	double timeR1 = getElapsedTimeMilliSec(event_read1);

	fprintf(stdout, "Write and que time for input buffer                        : %.6lf ms.\n", timeW1);
	fprintf(stdout, "Write and que time for output Buffer                       : %.6lf ms.\n", timeW2);
	fprintf(stdout, "Kernel time for Image Processing                           : %.6lf ms.\n", timeK1);
	fprintf(stdout, "Read from image output buffer                              : %.6lf ms.\n", timeR1);

	double timeGPUD = timeW1 + timeW2 + timeK1 + timeR1;

	fprintf(stdout, "---------------------------------------------------------------------------------\n");
	fprintf(stdout, "Total Time for computation of halfsized image on the GPU   : %.6lf ms. \n", timeGPUD);
	fprintf(stdout, "---------------------------------------------------------------------------------\n");

	return 0;

	// END OF EXECUTION
	//********************************************************************
}

//------------------------------------------------------------------------------------------------
// Helper function for profiling info for timing analysis
//
// Input Parameters :	const cl::Event& event - event for which time is measured
//								
// Return Parameters:	time elapsed in milliseconds
// 
//-------------------------------------------------------------------------------------------------
double getElapsedTimeMilliSec(const cl::Event& event) {
	cl_ulong nanoseconds = event.getProfilingInfo<CL_PROFILING_COMMAND_END>() - event.getProfilingInfo<CL_PROFILING_COMMAND_START>();
	return ((double)nanoseconds / 1000000);
}

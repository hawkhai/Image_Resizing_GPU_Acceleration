//============================================================================
// Includes
//============================================================================
#include <memory.h>
#include "tga_helper.h"

// Deprecated warnings suppressed, since currently working with OpenCL 1.X in my PC, with OpenCL 2.0 bindings	
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <CL\cl.hpp>

//============================================================================
// Declarations and Macro Defintions
//============================================================================

// TO BE CHNAGED AS PER USER NEEDS
#define TGA_DEBUG					0
#define TGA_PLATFORM_DEFAULT		1						// Set to 0 if to use TGA_PLATFORM_NAME platform, else the first platform taken
#define TGA_PLATFORM_NAME			"Intel(R) OpenCL"		// For example "Intel(R) OpenCL" or "AMD Accelerated Parallel Processing"
#define TGA_DEVICE_NUMBER			1						// Set device number. Numbering starts from 1


// Function Prototypes
double getElapsedTimeMilliSec(const cl::Event& event);
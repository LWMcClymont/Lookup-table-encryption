#define CL_USE_DEPRECATED_OPENCL_2_0_APIS	// using OpenCL 1.2, some functions deprecated in OpenCL 2.0
#define __CL_ENABLE_EXCEPTIONS				// enable OpenCL exemptions

// C++ standard library and STL headers
#include <iostream>
#include <vector>
#include <fstream>
#include <time.h>
#include "common.h"

#define GLOBAL_SIZE 512
#define LOCAL_SIZE 1
#define NUM_WORK_GROUPS GLOBAL_SIZE/LOCAL_SIZE

void upper(std::string * str);

int main(void) 
{
	cl::Platform platform;			// device's platform
	cl::Device device;				// device used
	cl::Context context;			// context for the device
	cl::Program program;			// OpenCL program object
	cl::Kernel kernel;				// a single kernel object
	cl::CommandQueue queue;			// commandqueue for a context and device

	cl::Buffer inputBuffer1, inputBuffer2, outputBuffer1;

	std::string filename = "plaintext.txt";

	std::vector <cl_uint> input;
	std::vector<cl_uint> output;

	std::string lookupTableString = "GXSQFAROWBLMTHCVPNZUIEYDKJ";
	std::string reverseLookupTableString = "FJOXVEANUZYKLRHQDGCMTPIBWS";

	std::vector<cl_uint> lookupTable(lookupTableString.length());
	std::vector<cl_uint> reverseLookupTable(reverseLookupTableString.length());

	std::copy(lookupTableString.begin(), lookupTableString.end(), lookupTable.begin());
	std::copy(reverseLookupTableString.begin(), reverseLookupTableString.end(), reverseLookupTable.begin());

	try {
		// select an OpenCL device
		if (!select_one_device(&platform, &device))
		{
			// if no device selected
			quit_program("Device not selected.");
		}

		// open file
		std::ifstream inputFile(filename);

		// check whether file was opened
		if (!inputFile.is_open())
		{
			std::cout << "File not found." << std::endl;
			return 0;
		}

		// ENCRYPT ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		// load file contents into string
		std::string inputString(std::istreambuf_iterator<char>(inputFile), (std::istreambuf_iterator<char>()));

		// convert string to uppercase
		upper(&inputString);

		// load string into vector
		input.resize(inputString.length());
		std::copy(inputString.begin(), inputString.end(), input.begin());

		output.resize(input.size());

		// create a context from device
		context = cl::Context(device);

		// build the program
		if(!build_program(&program, &context, "kernel.cl")) 
		{
			// if OpenCL program build error
			quit_program("OpenCL program build error.");
		}

		// create a kernel
		kernel = cl::Kernel(program, "task");

		// create command queue
		queue = cl::CommandQueue(context, device);

		// create buffers
		inputBuffer1 = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_uint) * input.size(), &input[0]);
		inputBuffer2 = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_uint) * lookupTable.size(), &lookupTable[0]);
		outputBuffer1 = cl::Buffer(context, CL_MEM_WRITE_ONLY, sizeof(cl_uint) * output.size());

		kernel.setArg(0, inputBuffer1);
		kernel.setArg(1, inputBuffer2);
		kernel.setArg(2, outputBuffer1);

		// enqueue kernel for execution
		cl::NDRange offset(0);
		cl::NDRange globalSize(GLOBAL_SIZE);
		cl::NDRange localSize(LOCAL_SIZE);

        queue.enqueueNDRangeKernel(kernel, offset, globalSize, localSize);

		// enqueue command to read from device to host memory
		queue.enqueueReadBuffer(outputBuffer1, CL_TRUE, 0, sizeof(cl_uint) * output.size(), &output[0]);

		// output results
		std::ofstream outputFile;
		outputFile.open("ciphertext.txt");

		for (cl_uint &c : output)
		{
			outputFile << (cl_char)c;
		}
		//
		////outputFile << programString;
		outputFile.close();

		// DECRYPT ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		input = output;

		// create buffers
		inputBuffer1 = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_uint) * input.size(), &input[0]);
		inputBuffer2 = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_uint) * reverseLookupTable.size(), &reverseLookupTable[0]);
		outputBuffer1 = cl::Buffer(context, CL_MEM_WRITE_ONLY, sizeof(cl_uint) * output.size());

		kernel.setArg(0, inputBuffer1);
		kernel.setArg(1, inputBuffer2);
		kernel.setArg(2, outputBuffer1);

		queue.enqueueNDRangeKernel(kernel, offset, globalSize, localSize);

		// enqueue command to read from device to host memory
		queue.enqueueReadBuffer(outputBuffer1, CL_TRUE, 0, sizeof(cl_uint) * output.size(), &output[0]);

		outputFile.open("decrypted.txt");

		for (cl_uint &c : output)
		{
			outputFile << (cl_char)c;
		}

		//outputFile << programString;
		outputFile.close();
	}
	// catch any OpenCL function errors
	catch (cl::Error e) {
		// call function to handle errors
		handle_error(e);
	}

#ifdef _WIN32
	// wait for a keypress on Windows OS before exiting
	std::cout << "\npress a key to quit...";
	std::cin.ignore();
#endif
	
	return 0;
}

void upper(std::string * str)
{
	for (char & c : *str)
	{
		// if the character is a lowercase character (ie not uppercase or special character)
		if (c >= 97 && c <= 122)
		{
			c = c - 32;
		}
	}
}

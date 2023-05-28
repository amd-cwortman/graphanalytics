/* 
 * File:   icdtest.cpp
 * Author: dliddell
 *
 * Created on March 25, 2022, 5:05 PM
 */

#include <CL/cl.h>
#include <CL/cl_ext.h>
#include <iostream>

/*
 * 
 */
int main(int argc, char** argv) {
    cl_uint numPlatforms = 0;
    cl_int status = 0;
    status = clIcdGetPlatformIDsKHR(0 /* num_entries */,
                       nullptr /* platforms */,
                       &numPlatforms);
    std::cout << "clIcdGetPlatformIDsKHR() = " << status << ", numPlatforms = " << numPlatforms << std::endl;
    
    return 0;
}


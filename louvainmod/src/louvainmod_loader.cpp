/*
 * Copyright 2020 Xilinx, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WANCUNCUANTIES ONCU CONDITIONS OF ANY KIND, either express or
 * implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Thanks to Aaron Isotton for his dynamic loading ideas in https://tldp.org/HOWTO/pdf/C++-dlopen.pdf

#include "xilinxlouvain.h"
#include <string>
#include <dlfcn.h>
#include <iostream>
#include <sstream>

const std::string xilinx_louvainmod_libName =
#ifdef XILINX_LOUVAINMOD_USE_STATIC_SO
        "libAMDLouvainStatic.so";
#else
        "libAMDLouvain.so";
#endif


#include "xilinx_apps_loader.hpp"


extern "C" {

#ifdef XILINX_LOUVAINMOD_INLINE_IMPL
#define XILINX_LOUVAINMOD_IMPL_DEF inline
#else
#define XILINX_LOUVAINMOD_IMPL_DEF
#endif
    
//XILINX_LOUVAINMOD_IMPL_DEF
//xilinx_apps::louvainmod::ImplBase *xilinx_louvainmod_createImpl(const xilinx_apps::louvainmod::Options& options,
//        unsigned valueSize)
//{
//    typedef xilinx_apps::louvainmod::ImplBase * (*CreateFunc)(const xilinx_apps::louvainmod::Options &, unsigned);
//    CreateFunc pCreateFunc = (CreateFunc) xilinx_apps::getDynamicFunction(xilinx_louvainmod_libName,
//        "xilinx_louvainmod_createImpl");
//    std::cout << "DEBUG: createImpl handle " << (void *) pCreateFunc << std::endl;
//    return pCreateFunc(options, valueSize);
//}
//
//XILINX_LOUVAINMOD_IMPL_DEF
//void xilinx_louvainmod_destroyImpl(xilinx_apps::louvainmod::ImplBase *pImpl) {
//    typedef xilinx_apps::louvainmod::ImplBase * (*DestroyFunc)(xilinx_apps::louvainmod::ImplBase *);
//    DestroyFunc pDestroyFunc = (DestroyFunc) xilinx_apps::getDynamicFunction(xilinx_louvainmod_libName,
//        "xilinx_louvainmod_destroyImpl");
//    pDestroyFunc(pImpl);
//}

XILINX_LOUVAINMOD_IMPL_DEF
int load_alveo_partitions(int argc, char *argv[]) {
    typedef int (*CreatePartitionsFunc)(int, char *[]);
    CreatePartitionsFunc pCreateFunc = (CreatePartitionsFunc) xilinx_apps::getDynamicFunction(xilinx_louvainmod_libName,
        "load_alveo_partitions");
    return pCreateFunc(argc, argv);
}

XILINX_LOUVAINMOD_IMPL_DEF
int create_and_load_alveo_partitions(int argc, char *argv[]) {
    typedef int (*CreatePartitionsFunc)(int, char *[]);
    CreatePartitionsFunc pCreateFunc = (CreatePartitionsFunc) xilinx_apps::getDynamicFunction(xilinx_louvainmod_libName,
        "create_and_load_alveo_partitions");
    return pCreateFunc(argc, argv);
}
/*
XILINX_LOUVAINMOD_IMPL_DEF
float loadAlveoProjectAndComputeLouvain(    
    char* xclbinPath, bool flowFast, unsigned numDevices, 
    char* alveoProject, 
    unsigned mode_zmq, unsigned numPureWorker, char* nameWorkers[128], unsigned nodeID,
    char* opts_outputFile, unsigned max_iter, unsigned max_level, float tolerence, 
    bool intermediateResult, bool verbose, bool final_Q, bool all_Q) 
{
    typedef float (*LoadPartitionsFunc)(char*, bool, unsigned, 
                                        char*, 
                                        unsigned, unsigned, char* [], unsigned,
                                        char*, unsigned, unsigned, float, 
                                        bool, bool, bool, bool);
    LoadPartitionsFunc pLoadFunc = (LoadPartitionsFunc) xilinx_apps::getDynamicFunction(xilinx_louvainmod_libName,
        "loadAlveoProjectAndComputeLouvain");
    return pLoadFunc(xclbinPath, flowFast, numDevices, 
                     alveoProject, 
                     mode_zmq, numPureWorker, nameWorkers, nodeID,
                     opts_outputFile, max_iter, max_level, tolerence, 
                     intermediateResult, verbose, final_Q, all_Q);
}
*/
}  // extern "C"



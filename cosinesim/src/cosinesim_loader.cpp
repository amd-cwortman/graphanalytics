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

#include "cosinesim.hpp"
#include <string>
#include <iostream>


const std::string xilinx_cosinesim_libName =
#ifdef XILINX_COSINESIM_USE_STATIC_SO
        "libAMDCosineSimStatic.so";
#else
        "libAMDCosineSim.so";
#endif

#include "xilinx_apps_loader.hpp"


extern "C" {

#ifdef XILINX_COSINESIM_INLINE_IMPL
#define XILINX_COSINESIM_IMPL_DEF inline
#else
#define XILINX_COSINESIM_IMPL_DEF
#endif
    
XILINX_COSINESIM_IMPL_DEF
xilinx_apps::cosinesim::ImplBase *xilinx_cosinesim_createImpl(const xilinx_apps::cosinesim::Options& options,
        unsigned valueSize)
{
    typedef xilinx_apps::cosinesim::ImplBase * (*CreateFunc)(const xilinx_apps::cosinesim::Options &, unsigned);
    std::cout << "INFO: Loading cosinesim shared library dynamically (via dlopen)." << std::endl;
    CreateFunc pCreateFunc = (CreateFunc) xilinx_apps::getDynamicFunction(xilinx_cosinesim_libName,
        "xilinx_cosinesim_createImpl");
//    std::cout << "DEBUG: createImpl handle " << (void *) pCreateFunc << std::endl;
    return pCreateFunc(options, valueSize);
}

XILINX_COSINESIM_IMPL_DEF
void xilinx_cosinesim_destroyImpl(xilinx_apps::cosinesim::ImplBase *pImpl) {
    typedef xilinx_apps::cosinesim::ImplBase * (*DestroyFunc)(xilinx_apps::cosinesim::ImplBase *);
    DestroyFunc pDestroyFunc = (DestroyFunc) xilinx_apps::getDynamicFunction(xilinx_cosinesim_libName,
        "xilinx_cosinesim_destroyImpl");
    pDestroyFunc(pImpl);
}

}  // extern "C"


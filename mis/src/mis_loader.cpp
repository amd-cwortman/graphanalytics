/*
 * Copyright 2021 Xilinx, Inc.
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

#include "xilinxmis.hpp"
#include <string>
#include <iostream>


const std::string xilinx_mis_libName =
#ifdef XILINX_MIS_USE_STATIC_SO
        "libAMDMisStatic.so";
#define XILINX_APPS_LOADER_USE_DLMOPEN 1
#else
        "libAMDMis.so";
#endif

#include "xilinx_apps_loader.hpp"

extern "C" {

#ifdef XILINX_MIS_INLINE_IMPL
#define XILINX_MIS_IMPL_DEF inline
#else
#define XILINX_MIS_IMPL_DEF
#endif
    
XILINX_MIS_IMPL_DEF
xilinx_apps::mis::ImplBase *xilinx_mis_createImpl(const xilinx_apps::mis::Options& options)
{
    typedef xilinx_apps::mis::ImplBase * (*CreateFunc)(const xilinx_apps::mis::Options &);
    std::cout << "INFO: Loading MIS API shared library " << xilinx_mis_libName << " dynamically (via dlopen)."
        << std::endl;
    CreateFunc pCreateFunc = (CreateFunc) xilinx_apps::getDynamicFunction(xilinx_mis_libName, "xilinx_mis_createImpl");
//    std::cout << "DEBUG: createImpl handle " << (void *) pCreateFunc << std::endl;
    return pCreateFunc(options);
}

XILINX_MIS_IMPL_DEF
void xilinx_mis_destroyImpl(xilinx_apps::mis::ImplBase *pImpl) {
    typedef void (*DestroyFunc)(xilinx_apps::mis::ImplBase *);
    DestroyFunc pDestroyFunc = (DestroyFunc) xilinx_apps::getDynamicFunction(xilinx_mis_libName,
        "xilinx_mis_destroyImpl");
    pDestroyFunc(pImpl);
}

}  // extern "C"
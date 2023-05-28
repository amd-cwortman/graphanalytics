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

#include <string>
#include <dlfcn.h>
#include <iostream>
#include <sstream>

#include "fuzzymatch.hpp"


const std::string xilinx_fuzzymatch_libName =
#ifdef XILINX_FUZZYMATCH_USE_STATIC_SO
        "libAMDFuzzyMatchStatic.so";
#else
        "libAMDFuzzyMatch.so";
#endif

#include "xilinx_apps_loader.hpp"


extern "C" {

#ifdef XILINX_FUZZYMATCH_INLINE_IMPL
#define XILINX_FUZZYMATCH_IMPL_DEF inline
#else
#define XILINX_FUZZYMATCH_IMPL_DEF
#endif
    
XILINX_FUZZYMATCH_IMPL_DEF
xilinx_apps::fuzzymatch::FuzzyMatchImpl *xilinx_fuzzymatch_createImpl(const xilinx_apps::fuzzymatch::Options& options)
{
    typedef xilinx_apps::fuzzymatch::FuzzyMatchImpl * (*CreateFunc)(const xilinx_apps::fuzzymatch::Options &);
    std::cout << "INFO: Loading fuzzymatch shared library dynamically (via dlopen)." << std::endl;
    CreateFunc pCreateFunc = (CreateFunc) xilinx_apps::getDynamicFunction(xilinx_fuzzymatch_libName,
        "xilinx_fuzzymatch_createImpl");
    std::cout << "DEBUG: createImpl handle " << (void *) pCreateFunc << std::endl;
    return pCreateFunc(options);
}

XILINX_FUZZYMATCH_IMPL_DEF
void xilinx_fuzzymatch_destroyImpl(xilinx_apps::fuzzymatch::FuzzyMatchImpl *pImpl) {
    typedef xilinx_apps::fuzzymatch::FuzzyMatchImpl * (*DestroyFunc)(xilinx_apps::fuzzymatch::FuzzyMatchImpl *);
    DestroyFunc pDestroyFunc = (DestroyFunc) xilinx_apps::getDynamicFunction(xilinx_fuzzymatch_libName,
        "xilinx_fuzzymatch_destroyImpl");
    pDestroyFunc(pImpl);
}

}  // extern "C"

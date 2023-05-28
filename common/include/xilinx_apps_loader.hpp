/*
 * Copyright 2021-2022 Xilinx, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Thanks to Aaron Isotton for his dynamic loading ideas in https://tldp.org/HOWTO/pdf/C++-dlopen.pdf


#ifndef XILINX_APPS_LOADER_H
#define XILINX_APPS_LOADER_H

#include <string>
#include <dlfcn.h>
#include <iostream>
#include <sstream>
#include <exception>

// Define this macro to use dlmopen instead of dlopen
//#define XILINX_APPS_LOADER_USE_DLMOPEN 1

// Define this macro to turn on debug messages
// #define XILINX_APPS_LOADER_DEBUG 1

namespace xilinx_apps {

/**
 * @brief %Exception class for shared library dynamic loading errors
 * 
 * This exception class is derived from `std::exception` and provides the standard @ref what() member function.
 * An object of this class is constructed with an error message string, which is stored internally and
 * retrieved with the @ref what() member function.
 */
class LoaderException : public std::exception {
    std::string message;
public:
    /**
     * Constructs an Exception object.
     * 
     * @param msg an error message string, which is copied and stored internal to the object
     */
    LoaderException(const std::string &msg) : message(msg) {}
    
    /**
     * Returns the error message string passed to the constructor.
     * 
     * @return the error message string
     */
    virtual const char* what() const noexcept override { return message.c_str(); }
};



inline void *getDynamicFunction(const std::string &libName, const std::string &funcName) {
#ifdef XILINX_APPS_LOADER_DEBUG
        std::cout << "DEBUG: xilinx_apps loader: libName=" << libName << ", funcName=" << funcName << std::endl;
#endif
    // open the library
    
    void *handle = 
#ifdef XILINX_APPS_LOADER_USE_DLMOPEN
        dlmopen(LM_ID_NEWLM, libName.c_str(), RTLD_LAZY);
#else
        dlopen(libName.c_str(), RTLD_LAZY | RTLD_GLOBAL);
#endif
    // To consider: save the handle for each libName in a map.  Calling dlopen multiple times increases the ref
    // count, requiring an equal number of dlclose, but in the context of an application that runs forever,
    // leaving the .so open doesn't matter.
        
#ifdef XILINX_APPS_LOADER_DEBUG
        std::cout << "DEBUG: xilinx_apps loader: after dlopen, handle=0x" << std::hex << handle << std::dec
            << std::endl;
#endif
        if (handle == nullptr) {
#ifdef XILINX_APPS_LOADER_DEBUG
            std::cout << "DEBUG: xilinx_apps loader: inside handle==nullptr" << std::endl;
#endif
            std::ostringstream oss;
            oss << "Cannot open library " << libName << ": " << dlerror()
                    << ".  Please ensure that the library's path is in LD_LIBRARY_PATH."  << std::endl;
#ifdef XILINX_APPS_LOADER_DEBUG
            std::cout << "DEBUG: xilinx_apps loader: after oss filling" << std::endl;
#endif
            throw LoaderException(oss.str());
        }

    // load the symbol
#ifdef XILINX_APPS_LOADER_DEBUG
    std::cout << "DEBUG: xilinx_apps loader: after handle==nullptr check" << std::endl;
#endif
    dlerror();  // reset errors
#ifdef XILINX_APPS_LOADER_DEBUG
    std::cout << "DEBUG: xilinx_apps loader: before dlsym" << std::endl;
#endif
    void *pFunc = dlsym(handle, funcName.c_str());
#ifdef XILINX_APPS_LOADER_DEBUG
    std::cout << "DEBUG: xilinx_apps loader: after dlsym" << std::endl;
#endif
    const char* dlsym_error2 = dlerror();
    if (dlsym_error2) {
#ifdef XILINX_APPS_LOADER_DEBUG
        std::cout << "DEBUG: xilinx_apps loader: inside dlsym_error2" << std::endl;
#endif
        std::ostringstream oss;
        oss << "Cannot load symbol '" << funcName << "': " << dlsym_error2
                << ".  Possibly an older version of library " << libName
                << " is in use.  Please install the correct version." << std::endl;
#ifdef XILINX_APPS_LOADER_DEBUG
        std::cout << "DEBUG: xilinx_apps loader: after 2nd oss filling" << std::endl;
#endif
        throw LoaderException(oss.str());
    }
#ifdef XILINX_APPS_LOADER_DEBUG
    std::cout << "DEBUG: xilinx_apps loader: before return" << std::endl;
#endif
    return pFunc;
}

}  // namespace xilinx_apps



#endif /* XILINX_APPS_LOADER_H */


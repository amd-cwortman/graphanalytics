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
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#pragma once

#ifndef _XF_GRAPH_L3_HANDLE_HPP_
#define _XF_GRAPH_L3_HANDLE_HPP_

#define XF_GRAPH_L3_MAX_DEVICES_PER_NODE 16   // maximu supported devices per node
#define XF_GRAPH_L3_SUCCESS 0
#define XF_GRAPH_L3_ERROR_CONFIG_FILE_NOT_EXIST -2
#define XF_GRAPH_L3_ERROR_XCLBIN_FILE_NOT_EXIST -3
#define XF_GRAPH_L3_ERROR_DLOPEN                -5
#define XF_GRAPH_L3_ERROR_NOT_ENOUGH_DEVICES    -6
#define XF_GRAPH_L3_ERROR_CU_NOT_SETUP          -7
#define XF_GRAPH_L3_ERROR_ALLOC_CU              -8
#define XF_GRAPH_L3_ERROR_DLSYM                 -9

#include "op_similaritydense.hpp"
#ifdef LOUVAINMOD
#include "op_louvainmodularity.hpp"
#endif

namespace xf {
namespace graph {
namespace L3 {
/**
 * @brief Graph library L3 handle
 *
 */
class Handle {
   public:
    /**
     * \brief setup information of single operation
     *
     * \param operationName operation's name
     * \param kernelName kernel's name, if kernelName is defined, kernelAlias will not be needed
     * \param kernelAlias kernel's name, if kernelName is not defined, kernelName will not be needed
     * \param requestLoad percentation of computing unit's occupation. Maximum is 100, which represents that 100% of the
     * computing unit are occupied. By using lower requestLoad, the hardware resources can be pipelined and higher
     * throughput can be achieved
     * \param xclbinFile xclbin file path
     * \param numDevices needed FPGA board number
     * \param deviceIDs FPGA board IDs
     *
     */
    struct singleOP {
        std::string operationName;           // for example, cosineSim
        std::string kernelName;              // user defined kernel names
        std::string kernelAlias;             // user defined kernel names
        unsigned int requestLoad = 100;
        std::string xclbinPath;              // xclbin full path
        unsigned int numDevices = 0;         // requested FPGA device number
        unsigned int cuPerBoard = 1;         // requested FPGA device number
        std::vector<unsigned int> deviceIDs; // deviceID
        void setKernelName(char* input) {
            std::string tmp = "";
            kernelName = input;
            kernelAlias = (char*)tmp.c_str();
        }
        void setKernelAlias(char* input) {
            std::string tmp = "";
            kernelName = (char*)tmp.c_str();
            kernelAlias = input;
        }
    };

    /**
     * \brief sparse matrix cosine/jaccard similarity operation
     *
     */
    //class opSimilaritySparse* opsimsparse;
    /**
     * \brief dense matrix cosine/jaccard similarity operation
     *
     */
    class opSimilarityDense* opsimdense;
    /**
     * \brief louvain modularity operation
     *
     */
    class opLouvainModularity* oplouvainmod;
    /**
     * \brief xilinx FPGA Resource Manager operation
     *
     */
    class openXRM* xrm;

    Handle() {
        opsimdense = new class opSimilarityDense;
#ifdef LOUVAINMOD
        oplouvainmod = new class opLouvainModularity;
#endif
        xrm = new class openXRM;
    };

    void free();

    void showHandleInfo();

    int setUp();

    void getEnv(std::vector<std::string> supportedDevices);

    void addOp(singleOP op);

    uint32_t getNumDevices() {return numDevices_;}

   private:
    uint32_t maxCU_;
    uint32_t numDevices_;
    uint32_t totalSupportedDevices_;
    uint32_t supportedDeviceIds_[XF_GRAPH_L3_MAX_DEVICES_PER_NODE];
    uint64_t maxChannelSize;
    std::vector<singleOP> ops;

    void loadXclbin(unsigned int deviceId, char* xclbinName);

    std::thread loadXclbinNonBlock(unsigned int deviceId, std::string& xclbinName);
    std::future<int> loadXclbinAsync(unsigned int deviceId, std::string& xclbinName);

    int32_t initOpSimDense(std::string kernelName,
                           std::string xclbinFile,
                           std::string kernelAlias,
                           unsigned int requestLoad,
                           unsigned int numDevices,
                           unsigned int cuPerBoard);

    int32_t initOpLouvainModularity(std::string kernelName,
                                    std::string xclbinFile,
                                    std::string kernelAlias,
                                    unsigned int requestLoad,
                                    unsigned int deviceNeeded,
                                    unsigned int cuPerBoard);
};
} // L3
} // graph
} // xf
#endif

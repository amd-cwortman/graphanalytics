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

#ifndef _XF_GRAPH_L3_OP_SIMILARITYDENSE_HPP_
#define _XF_GRAPH_L3_OP_SIMILARITYDENSE_HPP_

#include "graph.hpp"
#include "op_base.hpp"
#include "openclHandle.hpp"

namespace xf {
namespace graph {
namespace L3 {

class opSimilarityDense : public opBase {
   public:
    static uint32_t cuPerBoardSimDense;

    static uint32_t dupNmSimDense;

    std::thread simDenseThread;

    std::vector<event<int> > eventQueue;

    class clHandle* handles = nullptr;

    opSimilarityDense() : opBase() {};

    void setHWInfo(uint32_t numDevices, uint32_t maxCU);

    void freeSimDense(xrmContext* ctx);

    void init(class openXRM* xrm, std::string kernelName, std::string kernelAlias, 
              std::string xclbinFile,  uint32_t* deviceIDs, uint32_t* cuIDs, 
              unsigned int requestLoad);

    void initInt(class openXRM* xrm, char* kernelName, std::string kernelAlias, 
                 char* xclbinFile, uint32_t* deviceIDs, uint32_t* cuIDs, 
                 unsigned int requestLoad);

    void loadGraph(xf::graph::Graph<uint32_t, float> g); // loadGraph only support loading of CSR format graph

    void loadGraphMultiCardNonBlocking(int deviceID, int cuID, xf::graph::Graph<int32_t, int32_t> g);

    void loadGraphMultiCardBlocking(unsigned int deviceID, unsigned int cuID, xf::graph::Graph<int32_t, int32_t> g);

    static int computeInt(unsigned int deviceID,
                          unsigned int cuID,
                          unsigned int channelID,
                          xrmContext* ctx,
                          xrmCuResource* resR,
                          std::string instanceName,
                          clHandle* handles,
                          int32_t similarityType,
                          int32_t dataType,
                          int32_t sourceNUM,
                          int32_t* sourceWeight,
                          int32_t* sourceCoeffs,
                          int32_t topK,
                          xf::graph::Graph<int32_t, int32_t> g,
                          int32_t* resultID,
                          float* similarity);

    event<int> addworkInt(int32_t similarityType,
                          int32_t dataType,
                          int32_t sourceNUM,
                          int32_t* sourceWeight,
                          int32_t* sourceCoeffs,
                          int32_t topK,
                          xf::graph::Graph<int32_t, int32_t> g,
                          int32_t* resultID,
                          float* similarity);

   private:
    std::vector<int> deviceOffset;
    uint32_t numDevices_;
    uint32_t maxCU_;

    static void bufferInitInt(clHandle* hds,
                              std::string instanceName0,
                              xf::graph::Graph<int32_t, int32_t> g,
                              int cuID,
                              int similarityType,
                              int dataType,
                              int32_t topK,
                              int sourceNUM,
                              int32_t* sourceWeight,
                              int32_t* sourceCoeffs,
                              uint32_t* config,
                              int32_t* resultID,
                              float* similarity,
                              //cl::Kernel& kernel0,
                              std::vector<cl::Memory>& ob_in,
                              std::vector<cl::Memory>& ob_out);

    static void bufferInitIntDDR(clHandle* hds,
                              std::string instanceName0,
                              xf::graph::Graph<int32_t, int32_t> g,
                              int cuID,
                              int similarityType,
                              int dataType,
                              int32_t topK,
                              int sourceNUM,
                              int32_t* sourceWeight,
                              int32_t* sourceCoeffs,
                              uint32_t* config,
                              int32_t* resultID,
                              float* similarity,
                              //cl::Kernel& kernel0,
                              std::vector<cl::Memory>& ob_in,
                              std::vector<cl::Memory>& ob_out);

    static int cuExecute(
        clHandle* hds, cl::Kernel& kernel0, unsigned int num_runs, std::vector<cl::Event>* evIn, cl::Event* evOut);

    static void migrateMemObj(clHandle* hds,
                              bool type,
                              unsigned int num_runs,
                              std::vector<cl::Memory>& ob,
                              std::vector<cl::Event>* evIn,
                              cl::Event* evOut);

    static void cuRelease(xrmContext* ctx, xrmCuResource* resR);

    static void postProcessKNN(
        uint32_t topK, std::string* knownLabels, uint32_t* resultID, float* similarity, std::string* label);
};
} // L3
} // graph
} // xf

#endif

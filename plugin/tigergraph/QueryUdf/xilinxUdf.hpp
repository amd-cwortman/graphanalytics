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
/**
 * @file graph.hpp
 * @brief  This files contains graph definition.
 */

#ifndef _XILINXUDF_HPP_
#define _XILINXUDF_HPP_

#include "tgFunctions.hpp"
//#include "loader.hpp"
#include "codevector.hpp"
#include <algorithm>
// mergeHeaders 1 include start xilinxRecomEngine DO NOT REMOVE!
#include "cosinesim.hpp"
#include <cstdint>
#include <vector>
// mergeHeaders 1 include end xilinxRecomEngine DO NOT REMOVE!

// mergeHeaders 1 header start xilinxRecomEngine DO NOT REMOVE!
namespace xai {

using CosineSim = xilinx_apps::cosinesim::CosineSim<std::int32_t>;

class Context {
public:
    using IdMap = std::vector<std::uint64_t>;
    
private:
    unsigned numDevices_ = 1;
    xilinx_apps::cosinesim::ColIndex vectorLength_ = 0;
    bool isInitialized_ = false;
    CosineSim *pCosineSim_ = nullptr;
    IdMap idMap_;  // maps from vector ID to FPGA row number
public:
    static Context *getInstance() {
        static Context *s_pContext = nullptr;
        if (s_pContext == nullptr)
            s_pContext = new Context();
        return s_pContext;
    }
    
    Context() = default;
    ~Context() { delete pCosineSim_; }
    
    void setNumDevices(unsigned numDevices) {
        if (numDevices != numDevices_)
            clear();
        numDevices_ = numDevices;
    }
    
    unsigned getNumDevices() const { return numDevices_; }
    
    void setVectorLength(xilinx_apps::cosinesim::ColIndex vectorLength) {
        if (vectorLength != vectorLength_)
            clear();
        vectorLength_ = vectorLength;
    }
    
    xilinx_apps::cosinesim::ColIndex getVectorLength() const { return vectorLength_; }
    
    CosineSim *getCosineSimObj() {
        if (pCosineSim_ == nullptr) {
            xilinx_apps::cosinesim::Options options;
            options.vecLength = vectorLength_;
            options.devicesNeeded = numDevices_;
            pCosineSim_ = new CosineSim(options);
        }
        
        return pCosineSim_;
    }

    IdMap &getIdMap() { return idMap_; }
    
    void setInitialized() { isInitialized_ = true; }
    
    bool isInitialized() const { return isInitialized_; }

    void clear() {
        isInitialized_ = false;
        idMap_.clear();
        delete pCosineSim_;
        pCosineSim_ = nullptr;
    }
};
static CosineSim *getCosineSimObj(unsigned vecLength) {
    static CosineSim *s_cosineSimObj = nullptr;
    if (s_cosineSimObj)
    if (s_cosineSimObj == nullptr)
        s_cosineSimObj = new CosineSim();
}

}
// mergeHeaders 1 header end xilinxRecomEngine DO NOT REMOVE!

// Error codes from L3 starts from -1
// Error codes from UDF starts from -1001
#define XF_GRAPH_UDF_GRAPH_PARTITION_ERROR -1001

namespace UDIMPL {

/* Start Xilinx UDF additions */

inline double udf_bfs_fpga(int64_t sourceID,
                           ListAccum<int64_t>& offsetsList,
                           ListAccum<int64_t>& indicesList,
                           ListAccum<float>& weightsList,
                           ListAccum<int64_t>& predecent,
                           ListAccum<int64_t>& distance) {
    int numEdges = indicesList.size();
    int numVertices = offsetsList.size() - 1;
    uint32_t* predecentTmp = xf::graph::internal::aligned_alloc<uint32_t>(((numVertices + 15) / 16) * 16);
    uint32_t* distanceTmp = xf::graph::internal::aligned_alloc<uint32_t>(((numVertices + 15) / 16) * 16);
    memset(predecentTmp, -1, sizeof(uint32_t) * (((numVertices + 15) / 16) * 16));
    memset(distanceTmp, -1, sizeof(uint32_t) * (((numVertices + 15) / 16) * 16));
    xf::graph::Graph<uint32_t, uint32_t> g("CSR", numVertices, numEdges);

    int count = 0;
    while (count < numEdges) {
        if (count < offsetsList.size()) {
            int value0 = (int)(offsetsList.get(count));
            g.offsetsCSR[count] = value0;
        }
        int value = (int)(indicesList.get(count));
        float value1 = (float)(weightsList.get(count));
        g.indicesCSR[count] = value;
        g.weightsCSR[count] = 1;
        count++;
    }
    int res = bfs_fpga_wrapper(numVertices, numEdges, sourceID, g, predecentTmp, distanceTmp);

    for (int i = 0; i < numVertices; ++i) {
        if (predecentTmp[i] == (uint32_t)(-1)) {
            predecent += -1;
            distance += -1;
        } else {
            predecent += predecentTmp[i];
            distance += distanceTmp[i];
        }
    }
    g.freeBuffers();
    free(predecentTmp);
    free(distanceTmp);
    return res;
}

inline double udf_load_xgraph_fpga(ListAccum<int64_t>& offsetsList,
                                   ListAccum<int64_t>& indicesList,
                                   ListAccum<float>& weightsList) {
    int numEdges = indicesList.size();
    int numVertices = offsetsList.size() - 1;
    xf::graph::Graph<uint32_t, float> g("CSR", numVertices, numEdges);

    int count = 0;
    while (count < numEdges) {
        if (count < offsetsList.size()) {
            int value0 = (int)(offsetsList.get(count));
            g.offsetsCSR[count] = value0;
        }
        int value = (int)(indicesList.get(count));
        float value1 = (float)(weightsList.get(count));
        g.indicesCSR[count] = value;
        g.weightsCSR[count] = value1;
        count++;
    }
    int res = load_xgraph_fpga_wrapper(numVertices, numEdges, g);

    g.freeBuffers();
    return res;
}

inline double udf_shortest_ss_pos_wt_fpga(int64_t sourceID,
                                          int64_t numEdges,
                                          int64_t numVertices,
                                          ListAccum<int64_t>& predecent,
                                          ListAccum<float>& distance) {
    uint32_t length = ((numVertices + 1023) / 1024) * 1024;
    float** result;
    uint32_t** pred;
    result = new float*[1];
    pred = new uint32_t*[1];
    result[0] = xf::graph::internal::aligned_alloc<float>(length);
    pred[0] = xf::graph::internal::aligned_alloc<uint32_t>(length);
    memset(result[0], 0, length * sizeof(float));
    memset(pred[0], 0, length * sizeof(uint32_t));

    xf::graph::Graph<uint32_t, float> g("CSR", numVertices, numEdges);

    int res = shortest_ss_pos_wt_fpga_wrapper(numVertices, sourceID, 1, g, result, pred);

    for (int i = 0; i < numVertices; ++i) {
        predecent += pred[0][i];
        distance += result[0][i];
    }
    free(result[0]);
    free(pred[0]);
    delete[] result;
    delete[] pred;
    return res;
}

inline double udf_load_xgraph_pageRank_wt_fpga(ListAccum<int64_t>& offsetsList,
                                               ListAccum<int64_t>& indicesList,
                                               ListAccum<float>& weightsList) {
    int numEdges = indicesList.size();
    int numVertices = offsetsList.size() - 1;
    xf::graph::Graph<uint32_t, float> g("CSR", numVertices, numEdges);

    int count = 0;
    while (count < numEdges) {
        if (count < offsetsList.size()) {
            int value0 = (int)(offsetsList.get(count));
            g.offsetsCSR[count] = value0;
        }
        int value = (int)(indicesList.get(count));
        float value1 = (float)(weightsList.get(count));
        g.indicesCSR[count] = value;
        g.weightsCSR[count] = value1;
        count++;
    }
    int res = load_xgraph_pageRank_wt_fpga_wrapper(numVertices, numEdges, g);

    g.freeBuffers();
    return res;
}

inline double udf_pageRank_wt_fpga(
    int64_t numVertices, int64_t numEdges, float alpha, float tolerance, int64_t maxIter, ListAccum<float>& rank) {
    float* rankValue = new float[numVertices];

    xf::graph::Graph<uint32_t, float> g("CSR", numVertices, numEdges);

    int res = pageRank_wt_fpga_wrapper(alpha, tolerance, maxIter, g, rankValue);

    for (int i = 0; i < numVertices; ++i) {
        rank += rankValue[i];
    }
    delete[] rankValue;
    return res;
}

inline bool concat_uint64_to_str(string& ret_val, uint64_t val) {
    (ret_val += " ") += std::to_string(val);
    return true;
}

inline int64_t float_to_int_xilinx(float val) {
    return (int64_t)val;
}

inline int64_t udf_reinterpret_double_as_int64(double val) {
    int64_t double_to_int64 = *(reinterpret_cast<int64_t*>(&val));
    return double_to_int64;
}

inline double udf_reinterpret_int64_as_double(int64_t val) {
    double int64_to_double = *(reinterpret_cast<double*>(&val));
    return int64_to_double;
}

inline int64_t udf_lsb32bits(uint64_t val) {
    return val & 0x00000000FFFFFFFF;
}

inline int64_t udf_msb32bits(uint64_t val) {
    return (val >> 32) & 0x00000000FFFFFFFF;
}

inline VERTEX udf_getvertex(uint64_t vid) {
    return VERTEX(vid);
}

inline bool udf_setcode(int property, uint64_t startCode, uint64_t endCode, int64_t size) {
    return true;
}

inline bool udf_reset_timer(bool dummy) {
    xai::timer_start_time = std::chrono::high_resolution_clock::now();
    return true;
}

inline double udf_elapsed_time(bool dummy) {
    xai::t_time_point cur_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> l_durationSec = cur_time - xai::timer_start_time;
    double l_timeMs = l_durationSec.count() * 1e3;
    return l_timeMs;
}

inline double udf_cos_theta(ListAccum<int64_t> vec_A, ListAccum<int64_t> vec_B) {
    double res;
    int size = vec_A.size();
    int64_t norm_A = vec_A.get(0);
    double norm_d_A = *(reinterpret_cast<double*>(&norm_A));
    int64_t norm_B = vec_B.get(0);
    double norm_d_B = *(reinterpret_cast<double*>(&norm_B));
    double prod = 0;
    int i = xai::startPropertyIndex;
    while (i < size) {
        prod = prod + vec_A.get(i) * vec_B.get(i);
        ++i;
    }
    res = prod / (norm_d_A * norm_d_B);
    //std::cout << "val = " << res << std::endl;
    return res;
}

inline ListAccum<int64_t> udf_get_similarity_vec(int64_t property,
                                                 int64_t returnVecLength,
                                                 ListAccum<uint64_t>& property_vector) {
    ListAccum<uint64_t> result;
    int64_t size = property_vector.size();
    std::vector<uint64_t> codes;
    for (uint64_t val : property_vector) {
        codes.push_back(val);
    }
    std::vector<int> retcodes = xai::makeCosineVector(property, returnVecLength, codes);
    for (int value : retcodes) {
        result += value;
    }
    return result;
}

// mergeHeaders 1 body start xilinxRecomEngine DO NOT REMOVE!

inline int udf_xilinx_recom_set_num_devices(std::int64_t numDevices) {
    xai::Lock lock(xai::getMutex());
    xai::Context *pContext = xai::Context::getInstance();
    pContext->setNumDevices(unsigned(numDevices));
    return 0;
}

inline unsigned udf_xilinx_recom_get_num_devices() {
    xai::Lock lock(xai::getMutex());
    xai::Context *pContext = xai::Context::getInstance();
    return pContext->getNumDevices();
}

inline bool udf_xilinx_recom_is_initialized() {
    xai::Lock lock(xai::getMutex());
    xai::Context *pContext = xai::Context::getInstance();
    return pContext->isInitialized();
}

inline int udf_loadgraph_cosinesim_ss_fpga(int64_t numVertices,
                                           int64_t vecLength,
                                           ListAccum<ListAccum<int64_t> >& oldVectors,
                                           int devicesNeeded) {
    xai::Lock lock(xai::getMutex());
    xai::Context *pContext = xai::Context::getInstance();
    xai::Context::IdMap &idMap = pContext->getIdMap();
    idMap.clear();

    // If there are no vectors, consider the FPGA to be uninitialized
    
    xilinx_apps::cosinesim::RowIndex numVectors = xilinx_apps::cosinesim::RowIndex(oldVectors.size());
    if (numVectors < 1) {
        pContext->clear();
        return 0;
    }
    
    idMap.resize(numVectors, 0);
    xilinx_apps::cosinesim::ColIndex vectorLength = xilinx_apps::cosinesim::ColIndex(
            xilinx_apps::cosinesim::ColIndex(oldVectors.get(0).size()));
    pContext->setVectorLength(vectorLength);
    
    xai::CosineSim *pCosineSim = pContext->getCosineSimObj();
//    pCosineSim->openFpga();
    pCosineSim->startLoadPopulation();
    for (xilinx_apps::cosinesim::RowIndex vecNum = 0; vecNum < numVectors; ++vecNum) {
        ListAccum<int64_t> &curRowVec = oldVectors.get(vecNum);
        xilinx_apps::cosinesim::RowIndex rowIndex = 0;
        CosineSim::ValueType *pBuf = pCosineSim->getPopulationVectorBuffer(rowIndex);
        for (xilinx_apps::cosinesim::ColIndex eltNum = 3; eltNum < vectorLength; ++eltNum)
            *pBuf++ = CosineSim::ValueType(curRowVec.get(eltNum));
        pCosineSim->finishCurrentPopulationVector();
        uint64_t vertexId = ((curRowVec.get(2) << 32) & 0xFFFFFFF00000000) | (curRowVec.get(1) & 0x00000000FFFFFFFF);
        idMap[vecNum] = vertexId;
    }
    pCosineSim->finishLoadPopulationVectors();
    return 0;
}

// Enable this to print profiling messages to the log (via stdout)
#define XILINX_RECOM_PROFILE_ON

inline ListAccum<testResults> udf_cosinesim_ss_fpga(int64_t topK,
    int64_t numVertices, int64_t vecLength, ListAccum<int64_t>& newVector,
    int devicesNeeded)
{
    xai::Lock lock(xai::getMutex());
    ListAccum<testResults> result;
    xai::Context *pContext = xai::Context::getInstance();

    if (!pContext->isInitialized())
        return result;
    
    xai::Context::IdMap &idMap = pContext->getIdMap();

//    std::cout << "DEBUG: " << __FILE__ << "::" << __FUNCTION__
//            << " numVertices=" << numVertices << ", general=" << general 
//            << ", rest=" << rest 
//            << ", split=" << devicesNeeded * cuNm * splitNm << std::endl;

    //-------------------------------------------------------------------------
#ifdef XILINX_RECOM_PROFILE_ON
    std::chrono::time_point<std::chrono::high_resolution_clock> l_start_time =
            std::chrono::high_resolution_clock::now();
    std::cout << "PROFILING: " << __FILE__ << "::" << __FUNCTION__ 
              << " preprocessing start=" << l_start_time.time_since_epoch().count() 
              << std::endl;
#endif
    //-------------------------------------------------------------------------

    std::vector<xai::CosineSim::ValueType> nativeTargetVector;
    const xilinx_apps::cosinesim::ColIndex vectorLength = pContext->getVectorLength();
    nativeTargetVector.reserve(vectorLength);
    for (xilinx_apps::cosinesim::ColIndex eltNum = 3; eltNum < vectorLength; ++eltNum)
        nativeTargetVector.push_back(newVector.get(eltNum));
    xai::CosineSim *pCosineSim = pContext->getCosineSimObj();

    //---------------------------------------------------------------------------
#ifdef XILINX_RECOM_PROFILE_ON
    std::chrono::time_point<std::chrono::high_resolution_clock> l_end_time =
            std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> l_durationSec = l_end_time - l_start_time;
    double l_timeMs = l_durationSec.count() * 1e3;
    std::cout << "PROFILING: " << __FILE__ << "::" << __FUNCTION__ 
              << " preprocessing runtime msec=  " << std::fixed << std::setprecision(6) 
              << l_timeMs << std::endl;
#endif
    //---------------------------------------------------------------------------

    //---------------- Run L3 API -----------------------------------
    //-------------------------------------------------------------------------
#ifdef XILINX_RECOM_PROFILE_ON
    std::chrono::time_point<std::chrono::high_resolution_clock> l_start_time1 =
            std::chrono::high_resolution_clock::now();
    std::cout << "PROFILING: " << __FILE__ << "::" << __FUNCTION__ 
              << " cosinesim_ss_dense_fpga start=" << l_start_time1.time_since_epoch().count() << std::endl;
#endif
    //-------------------------------------------------------------------------

    std::vector<xilinx_apps::cosinesim::Result> apiResults
            = pCosineSim->matchTargetVector(topK, nativeTargetVector.data());
    
    //---------------------------------------------------------------------------
#ifdef XILINX_RECOM_PROFILE_ON
    std::chrono::time_point<std::chrono::high_resolution_clock> l_end_time1 =
            std::chrono::high_resolution_clock::now();
    l_durationSec = l_end_time1 - l_start_time1;
    l_timeMs = l_durationSec.count() * 1e3;
    std::cout << "PROFILING: " << __FILE__ << "::" << __FUNCTION__ 
              << " cosinesim_ss_dense_fpga runtime msec=  " << std::fixed << std::setprecision(6) 
              << l_timeMs << std::endl;
#endif
    //---------------------------------------------------------------------------

    //-------------------------------------------------------------------------
#ifdef XILINX_RECOM_PROFILE_ON
    std::chrono::time_point<std::chrono::high_resolution_clock> l_start_time2 =
            std::chrono::high_resolution_clock::now();
    std::cout << "PROFILING: " << __FILE__ << "::" << __FUNCTION__ 
              << " postprocessing start=" << l_start_time2.time_since_epoch().count() << std::endl;
#endif
    //-------------------------------------------------------------------------

    for (xilinx_apps::cosinesim::Result &apiResult : apiResults) {
        if (apiResult.index_ < 0 || apiResult.index_ >= idMap.size())
            continue;
        result += testResults(VERTEX(idMap[apiResult.index_]), apiResult.similarity_);
    }

    //---------------------------------------------------------------------------
#ifdef XILINX_RECOM_PROFILE_ON
    std::chrono::time_point<std::chrono::high_resolution_clock> l_end_time2 =
            std::chrono::high_resolution_clock::now();
    l_durationSec = l_end_time2 - l_start_time2;
    l_timeMs = l_durationSec.count() * 1e3;
    std::cout << "PROFILING: " << __FILE__ << "::" << __FUNCTION__ 
              << " postprocessing runtime msec=  " << std::fixed << std::setprecision(6) 
              << l_timeMs << std::endl;
#endif
    //---------------------------------------------------------------------------

    return result;
}

/* End Xilinx Cosine Similarity Additions */
// mergeHeaders 1 body end xilinxRecomEngine DO NOT REMOVE!

}

#endif /* EXPRFUNCTIONS_HPP_ */

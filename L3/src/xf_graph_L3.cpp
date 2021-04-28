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


#ifndef _XF_GRAPH_L3_CPP_
#define _XF_GRAPH_L3_CPP_

#include "xf_graph_L3.hpp"

namespace xf {
namespace graph {
namespace L3 {

int runMultiEvents(uint32_t number, std::vector<xf::graph::L3::event<int> >& f) {
    int ret = 0;
    for (uint32_t i = 0; i < number; ++i) {
        ret += f[i].wait();
    }
    return ret;
}

event<int> cosineSimilaritySSDense(xf::graph::L3::Handle& handle,
                                   uint32_t sourceNUM,
                                   uint32_t* sourceWeights,
                                   uint32_t topK,
                                   xf::graph::Graph<uint32_t, float> g,
                                   uint32_t* resultID,
                                   float* similarity) {
    (handle.opsimdense)
        ->eventQueue.push_back(
            (handle.opsimdense)->addwork(1, 1, sourceNUM, sourceWeights, topK, g, resultID, similarity));
    std::packaged_task<int(uint32_t, std::vector<event<int> >&)> t(runMultiEvents);
    std::future<int> f0 = t.get_future();
    (handle.opsimdense)->simDenseThread = std::thread(std::move(t), 1, std::ref((handle.opsimdense)->eventQueue));
    return event<int>(std::forward<std::future<int> >(f0));
};

std::vector<event<int> > cosineSimilaritySSDenseMultiCard(xf::graph::L3::Handle& handle,
                                                          int32_t deviceNm,
                                                          int32_t sourceNUM,
                                                          int32_t* sourceWeights,
                                                          int32_t topK,
                                                          xf::graph::Graph<int32_t, int32_t>** g,
                                                          int32_t** resultID,
                                                          float** similarity) {
    std::vector<event<int> > eventQueue;
    for (int i = 0; i < deviceNm; ++i) {
        eventQueue.push_back(
            (handle.opsimdense)->addworkInt(1, 0, sourceNUM, sourceWeights, topK, g[i][0], resultID[i], similarity[i]));
    }
    return eventQueue;
};

int cosineSimilaritySSDenseMultiCardBlocking(xf::graph::L3::Handle& handle,
                                             int32_t deviceNm,
                                             int32_t sourceNUM,
                                             int32_t* sourceWeights,
                                             int32_t topK,
                                             xf::graph::Graph<int32_t, int32_t>** g,
                                             int32_t* resultID,
                                             float* similarity) {
    std::vector<event<int> > eventQueue;
    float** similarity0 = new float*[deviceNm];
    int32_t** resultID0 = new int32_t*[deviceNm];
    int counter[deviceNm];
    for (int i = 0; i < deviceNm; ++i) {
        counter[i] = 0;
        similarity0[i] = aligned_alloc<float>(topK);
        resultID0[i] = aligned_alloc<int32_t>(topK);
        memset(resultID0[i], 0, topK * sizeof(int32_t));
        memset(similarity0[i], 0, topK * sizeof(float));
    }
    for (int i = 0; i < deviceNm; ++i) {
        eventQueue.push_back(
            (handle.opsimdense)
                ->addworkInt(1, 0, sourceNUM, sourceWeights, topK, g[i][0], resultID0[i], similarity0[i]));
    }
    int ret = 0;
    for (uint32_t i = 0; i < eventQueue.size(); ++i) {
        ret += eventQueue[i].wait();
    }
    for (int i = 0; i < topK; ++i) {
        similarity[i] = similarity0[0][counter[0]];
        int32_t prev = 0;
        resultID[i] = resultID0[0][counter[0]];
        counter[0]++;
        for (int j = 1; j < deviceNm; ++j) {
            if (similarity[i] < similarity0[j][counter[j]]) {
                similarity[i] = similarity0[j][counter[j]];
                resultID[i] = resultID0[j][counter[j]];
                counter[prev]--;
                prev = j;
                counter[j]++;
            }
        }
    }
    for (int i = 0; i < deviceNm; ++i) {
        free(similarity0[i]);
        free(resultID0[i]);
    }
    delete[] similarity0;
    delete[] resultID0;
    return ret;
};

#ifdef LOUVAINMOD
void louvainModularity(xf::graph::L3::Handle& handle,
					   int flowMode,
                       GLV* glv,
                       GLV* pglv,
                       bool opts_coloring,
                       long opts_minGraphSize,
                       double opts_threshold,
                       double opts_C_thresh,
                       int numThreads) 
{

    std::cout << "DEBUG: " << __FUNCTION__ << "handle=" << &handle << std::endl;

    // auto ev = (handle.oplouvainmod)
    //              ->addwork(glv, opts_C_thresh, buff_cl, buff_host, kernel_louvain, q, eachItrs, currMod,
    //              eachTimeInitBuff, eachTimeReadBuff);
    (handle.oplouvainmod)
        ->demo_par_core(0, flowMode, glv, pglv, opts_coloring, opts_minGraphSize, opts_threshold, opts_C_thresh, numThreads);
};
#endif

} // L3
} // graph
} // xf
#endif

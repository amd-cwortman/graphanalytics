/*
 * Copyright 2020-2021 Xilinx, Inc.
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

#ifndef XILINX_MIS_HPP
#define XILINX_MIS_HPP

// mergeHeaders 1 name xilinxMis
// mergeHeaders 1 section include start xilinxMis DO NOT REMOVE!
#include "xilinxMisImpl.hpp"
#include <cstdint>
#include <vector>
#include <random>
#include <iostream>
// mergeHeaders 1 section include end xilinxMis DO NOT REMOVE!

namespace UDIMPL {

// mergeHeaders 1 section body start xilinxMis DO NOT REMOVE!
inline int64_t rand_int (int minVal, int maxVal) {
    std::random_device rd;
    std::mt19937 e1(rd());
    std::uniform_int_distribution<int> dist(minVal, maxVal);
    return (int64_t) dist(e1);
}

inline void udf_reset_context() {
    xilMis::Lock guard(xilMis::getMutex());
    xilMis::Context *context = xilMis::Context::getContext();
    context->resetContext();
}

inline int udf_get_next_vid(int tg_v_id) {
    xilMis::Lock guard(xilMis::getMutex());
    xilMis::Context *context = xilMis::Context::getContext();
    context->addVertexToMap(tg_v_id);
    return context->getNextVid();
}

inline void udf_build_row_ptr(ListAccum<int> row_ptr)
{
    xilMis::Context *context = xilMis::Context::getContext();
    for(int i = 0; i < row_ptr.size(); i++)
        context->addRowPtrEntry(row_ptr.get(i));
}

inline void udf_build_col_idx(ListAccum<int> neighbors)
{
    xilMis::Context *context = xilMis::Context::getContext();
    for(int i = 0; i < neighbors.size(); i++)
        context->addColIdxEntry(neighbors.get(i));
}

inline void udf_dump_csr()
{
    xilMis::Context *context = xilMis::Context::getContext();
    std::vector<int> rowPtr = context->getRowPtr();
    std::vector<int> colIdx = context->getColIdx();

    std::ofstream row_file ("/tmp/rowPtr.bin", std::ios::out | std::ios::binary);
    row_file.write(reinterpret_cast<const char*>(&rowPtr[0]), rowPtr.size()*4);
    row_file.close();

    std::ofstream col_file ("/tmp/colIdx.bin", std::ios::out | std::ios::binary);
    col_file.write(reinterpret_cast<const char*>(&colIdx[0]), colIdx.size()*4);
    col_file.close();

    std::ofstream info_file ("/tmp/info.txt", std::ios::out);
    info_file << "graph\n";
    info_file << rowPtr.size()-1 << std::endl;
    info_file << rowPtr.size()-1 << std::endl;
    info_file << colIdx.size() << std::endl;
    info_file.close();
}

inline ListAccum<ListAccum<VERTEX> > udf_xilinx_mis(int num_schedules, int num_verts, std::string &status)
{
    ListAccum<ListAccum<VERTEX> > scheduleAccum;
    ListAccum<VERTEX> misAccum;
    int total_scheduled;
    xilMis::Context *context = xilMis::Context::getContext();

    // CSR data check
    status = context->checkContext(num_verts);
    if(status != "0") return scheduleAccum;

    // build/get MIS object
    xilinx_apps::mis::MIS *pMis = context->getMisObj();

    // set MIS object
    context->setMisGraph();

    // execute MIS
    auto schedules = pMis->executeMIS(num_schedules);

    // collect results into TG accumulator
    for (int i = 0; i < schedules.size(); i++ ) {
        // add to lists
        for(int &vid : schedules[i]) {
            misAccum += VERTEX(context->v_id_map[vid]);
        }

        scheduleAccum += misAccum;
        misAccum.clear();
    }

    return scheduleAccum;
}

// mergeHeaders 1 section body end xilinxMis DO NOT REMOVE!

}

#endif /* XILINX_MIS_HPP */
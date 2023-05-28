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
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef CODEVECTOR_HPP
#define CODEVECTOR_HPP

#include "xilinxRecomEngineImpl.hpp"
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <unordered_map>
#include <vector>
#include <cmath>
#include <chrono>

namespace logSimilarityDemo {

using Mutex = xilRecom::Mutex;
using Lock = xilRecom::Lock;

typedef std::chrono::time_point<std::chrono::high_resolution_clock> t_time_point, *pt_time_point; 
//extern t_time_point timer_start_time;

inline t_time_point &getTimerStartTime() {
    static t_time_point s_startTime;
    return s_startTime;
}

//class CodeToIdMap;
//extern CodeToIdMap* pMap;

const unsigned int startPropertyIndex = 3; // Start index of property in the
                                           // vector, 0, 1 and 2 are ressrved
                                           // for norm, id */

//extern std::vector<uint64_t> IDMap;

typedef std::int32_t CosineVecValue; ///< A value for an element of a cosine similarity vector

// Standard values for cosine vector elements
/*
#ifdef UNIT_TEST
const CosineVecValue MaxVecValue = 11000;
const CosineVecValue MinVecValue = 10000;
const CosineVecValue NullVecValue = -10000;
#else
const CosineVecValue MaxVecValue = 10480;
const CosineVecValue MinVecValue = MaxVecValue / 2;
const CosineVecValue NullVecValue = -MaxVecValue;
#endif
*/

inline std::vector<CosineVecValue> buildGloVeEmbedding(const std::string msg) {
    // place holder dummy embedding
    int global_vec_len = 50;
    std::vector<CosineVecValue> result (global_vec_len);
    for(int i=0; i < global_vec_len; i++) {
        result[i] = 1;
    }
    return result;
}


} // namespace

#endif /* CODEVECTOR_HPP */

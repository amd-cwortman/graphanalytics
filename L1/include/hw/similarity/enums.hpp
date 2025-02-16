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
#ifndef __XF_GRAPH_ENUMS_HPP_
#define __XF_GRAPH_ENUMS_HPP_

#include <ap_int.h>

namespace xf {

namespace graph {

namespace enums {
enum SimilarityType {
    JACCARD_SIMILARITY = 0,
    COSINE_SIMILARITY = 1,
};

enum GraphType { SPARSE = 0, DENSE = 1 };

enum DataType { UINT32_TYPE = 0, FLOAT_TYPE = 1, UINT64_TYPE = 2, DOUBLE_TYPE = 3, INT32_TYPE = 4, INT64_TYPE = 5 };

} // enums

} // graph

} // xf

#endif

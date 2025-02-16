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

#ifndef XILINX_COM_DETECT_HPP
#define XILINX_COM_DETECT_HPP

// mergeHeaders 1 name xilinxComDetect
// mergeHeaders 1 section include start xilinxComDetect DO NOT REMOVE!
#include "xilinxComDetectImpl.hpp"
#include <cstdint>
#include <vector>
// mergeHeaders 1 section include end xilinxComDetect DO NOT REMOVE!

namespace UDIMPL {

// mergeHeaders 1 section body start xilinxComDetect DO NOT REMOVE!

inline void udf_com_detect_print_version() {
    // Feel free to change the revision number whenever you want to be sure that your code changes
    // actually made it into the TG installation!
    std::cout << "INFO: comdetect UDF version 0.1.3 built " << __DATE__ << ' ' << __TIME__ << ", "
#ifdef XILINX_COM_DETECT_DEBUG_ON
        "DEBUG MESSAGES ON"
#else
        "debug messages off"
#endif
        << std::endl;
}


inline void udf_reset_nextId() {

    xilComDetect::Context *pContext = xilComDetect::Context::getInstance();
    std::cout << "nextId_ = " << pContext->getNextId() <<std::endl;
    pContext->clearPartitionData();
}

inline uint64_t udf_get_nextId(uint64_t out_degree){
    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
    xilComDetect::Context *pContext = xilComDetect::Context::getInstance();
    //pContext->addDegreeList((long)out_degree);
    pContext->curParPtr->degree_list.push_back(out_degree);
#ifdef  XILINX_COM_DETECT_DUMP_GRAPH
    std::cout << " louvainId = " << pContext->getNextId() <<" out_degree = " << out_degree <<std::endl;
#endif
    uint64_t nextId = pContext->getNextId();
    uint64_t nextIdIncr = nextId +1 ;
    pContext->setNextId(nextIdIncr);
    return nextId;
}

inline uint64_t udf_get_partition_size(){
    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
     xilComDetect::Context *pContext = xilComDetect::Context::getInstance();
#ifdef XILINX_COM_DETECT_DEBUG_ON
     std::cout << "Partition Size = " << pContext->getNextId()<<std::endl;
#endif
     return pContext->getNextId();
}
inline int udf_xilinx_comdetect_set_node_id(uint nodeId)
{
    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
    std::cout << "DEBUG: " << __FUNCTION__ << " nodeId=" << nodeId << std::endl;
    xilComDetect::Context *pContext = xilComDetect::Context::getInstance();

    pContext->setNodeId(unsigned(nodeId));
    return nodeId;
}
//add new
inline uint64_t udf_get_global_louvain_id(uint64_t louvain_id){
    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
    xilComDetect::Context *pContext = xilComDetect::Context::getInstance();
    return pContext->getLouvainOffset() + louvain_id ;
}

inline void udf_set_louvain_offset(uint64_t louvain_offset){
    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
    xilComDetect::Context *pContext = xilComDetect::Context::getInstance();
#ifdef XILINX_COM_DETECT_DUMP_GRAPH
    std::cout << "Louvain Offsets = " << louvain_offset <<std::endl;
#endif
    pContext->setLouvainOffset(louvain_offset);
}


inline void udf_start_whole_graph_collection() {
#ifdef XILINX_COM_DETECT_DEBUG_ON
    std::cout << "AGML-DEBUG: " << __FUNCTION__ << std::endl;
#endif
    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
    xilComDetect::Context *pContext = xilComDetect::Context::getInstance();
    pContext->clearPartitionData();
    pContext->curParPtr= new xilComDetect::Partitions();
    //pContext->curParPtr->vertexMap.clear();
#ifdef XILINX_COM_DETECT_DUMP_MTX
    pContext->mtxFstream.open("/proj/gdba/xgstore/tg.mtx");
#endif
#ifdef XILINX_COM_DETECT_DEBUG_ON
    std::cout << "AGML-DEBUG: End " << __FUNCTION__ << std::endl;
#endif
}

// Adds one vertex to a temporary map of vertices to be sorted in udf_process_whole_graph_vertices.
//
inline void udf_add_whole_graph_vertex(uint64_t vertexId, uint64_t outDegree) {
#ifndef XILINX_COM_DETECT_STUBS_ONLY
    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
    xilComDetect::Context *pContext = xilComDetect::Context::getInstance();
    pContext->curParPtr->vertexMap[vertexId] = xilComDetect::LouvainVertex(outDegree);
#ifdef XILINX_COM_DETECT_DEBUG_ON
    if (pContext->curParPtr->vertexMap.size() % 1000000 == 0)
        std::cout << "AGML-DEBUG: " << __FUNCTION__ << " processed " << pContext->curParPtr->vertexMap.size() << " vertices." << std::endl;
#endif
#endif
}

// From the temporary map of vertices collected by udf_add_whole_graph_vertex, produces an outbound edge degree list
// sorted in ascending order of prenumbered (.mtx) IDs.  Also generates a map from prenumbered ID to "compressed ID."
// Assuming that the prenumbered IDs are not contiguous, each prenumbered ID is mapped to such a contiguous ID
// starting from 0.  For example, the prenumbered IDs 5, 8, 10 are mapped to compressed IDs 0, 1, 2.
//
inline int udf_process_whole_graph_vertices() 
{
#ifdef XILINX_COM_DETECT_DEBUG_ON
    std::cout << "AGML-DEBUG: Start " << __FUNCTION__ << std::endl;
#endif    
#ifndef XILINX_COM_DETECT_STUBS_ONLY
    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
    xilComDetect::Context *pContext = xilComDetect::Context::getInstance();
    std::cout << "AGML-INFO: " << __FUNCTION__ << " Vertex count=" << pContext->curParPtr->vertexMap.size() << std::endl;

    if (pContext->curParPtr->vertexMap.size() == 1) {
        std::cout << "AGML-ERROR: The number of vertices is too small. Double check if vertex ID is loaded with sequential index." << std::endl;
        return -1;
    }
    uint64_t id = 0;
    pContext->curParPtr->degree_list.reserve(pContext->curParPtr->vertexMap.size());
#ifdef XILINX_COM_DETECT_DUMP_MTX
    pContext->mtxFstream << "*Vertices " << pContext->vertexMap.size() << std::endl;
    pContext->mtxFstream << "*Edges 0" << std::endl;  // We're not going to add a query just to pre-count the edges
#endif
    for (auto &entry : pContext->curParPtr->vertexMap) {
        pContext->curParPtr->degree_list.push_back(entry.second.outDegree_);
        entry.second.id_ = id++;
    }
    pContext->setNextId(id);
#endif
#ifdef XILINX_COM_DETECT_DEBUG_ON
    std::cout << "AGML-DEBUG: End " << __FUNCTION__ << std::endl;
#endif

    return 0;
}

// Given a prenumbered (.mtx) ID for a vertex, returns the compressed ID for that vertex.
//
inline uint64_t udf_get_compressed_id(uint64_t prenumberedId) {
    // No mutex locking!  This function performs only R/O accesses to finished data structures
    xilComDetect::Context *pContext = xilComDetect::Context::getInstance();
    auto it = pContext->curParPtr->vertexMap.find(prenumberedId);
    return (it == pContext->curParPtr->vertexMap.end()) ? 0 : it->second.id_;
}


inline void udf_start_partition(const std::string &prj_pathname, const std::string &graph_name, int numVertices,
    bool useLowBwPartitioner)
{
#ifdef XILINX_COM_DETECT_DEBUG_ON
    std::cout << "DEBUG: " << __FUNCTION__ << std::endl;
#endif
#ifndef XILINX_COM_DETECT_STUBS_ONLY
    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
    xilComDetect::Context *pContext = xilComDetect::Context::getInstance();
    std::cout << "DEBUG: before alveo_parj_path" << std::endl;
    if(prj_pathname.empty()) {
        pContext->setAlveoProject(graph_name);
        pContext->setPartitionNameMode(xilComDetect::PartitionNameMode::Auto);
    } else {
        pContext->setAlveoProjectRaw(prj_pathname);
        pContext->setPartitionNameMode(xilComDetect::PartitionNameMode::Flat);
    }
    xilinx_apps::louvainmod::LouvainMod *pLouvainMod = pContext->getLouvainModObj();
    xilinx_apps::louvainmod::LouvainMod::PartitionOptions options; //use default value
    options.totalNumVertices = numVertices;
    options.LBW_partition = useLowBwPartitioner;
#ifdef XILINX_COM_DETECT_DEBUG_ON
    std::cout << "DEBUG: totalNumVertices: " << numVertices << std::endl;
#endif
    pLouvainMod->startPartitioning(options);
#endif
#ifdef XILINX_COM_DETECT_DEBUG_ON
    std::cout << "DEBUG: End " << __FUNCTION__ << std::endl;
#endif
}



inline void udf_save_EdgePtr( ){
#ifdef XILINX_COM_DETECT_DEBUG_ON
    std::cout << "AGML-DEBUG: " << __FUNCTION__ << std::endl;
#endif
#ifndef XILINX_COM_DETECT_STUBS_ONLY
    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
    xilComDetect::Context *pContext = xilComDetect::Context::getInstance();
     //build offsets_tg

    pContext->curParPtr->mEdgePtrVec.push_back(0);
    for(int i = 1;i <= pContext->curParPtr->degree_list.size();i++){
        pContext->curParPtr->mEdgePtrVec.push_back(pContext->curParPtr->degree_list[i-1] + pContext->curParPtr->mEdgePtrVec[i-1]); //offsets_tg[i] += pContext->offsets_tg[i-1];
    }

     pContext->setOffsetsTg(pContext->curParPtr->mEdgePtrVec.data());
     int size= pContext->curParPtr->mEdgePtrVec.size();
     pContext->curParPtr->mEdgeVec.resize(pContext->curParPtr->mEdgePtrVec[size-1]); //NE
     pContext->curParPtr->mDgrVec.resize(pContext->curParPtr->mEdgePtrVec[size-1]); //NE
     pContext->curParPtr->addedOffset.resize(pContext->curParPtr->mEdgePtrVec.size()); //NV+1
     std::fill(pContext->curParPtr->addedOffset.begin(), pContext->curParPtr->addedOffset.end(),0);
     //std::cout<< "DEBUG:: mDgrVec SIZE:" << pContext->mDgrVec.size() <<" ;"<< pContext->mEdgePtrVec[size-1] << std::endl;
#endif
#ifdef XILINX_COM_DETECT_DEBUG_ON
    std::cout << "AGML-DEBUG: End " << __FUNCTION__ << " NE=" << pContext->curParPtr->mEdgePtrVec[size-1] << std::endl;
#endif
}

inline void udf_set_louvain_edge_list( uint64_t louvainIdSource, uint64_t louvainIdTarget, float wtAttr, uint64_t outDgr) 
{
#ifndef XILINX_COM_DETECT_STUBS_ONLY
    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
    xilComDetect::Context *pContext = xilComDetect::Context::getInstance();
    const long idx = louvainIdSource - pContext->getLouvainOffset();
    const long startingOffset = pContext->curParPtr->mEdgePtrVec[idx];
    const long newEdgeOffset = startingOffset +  pContext->curParPtr->addedOffset[idx];

#ifdef XILINX_COM_DETECT_DEBUG_ON
    if (pContext->curParPtr->edgeCount == 0)
        std::cout << "DEBUG: " << __FUNCTION__ << " start first edge" << std::endl;

    // Make sure the new edge fits in the space allocated
    if (newEdgeOffset >= long(pContext->curParPtr->mEdgeVec.size()))
        std::cout << "DEBUG: " << __FUNCTION__ << " edge vector overflow.  mEdgeVec.size()="
            << pContext->curParPtr->mEdgeVec.size() << ", newEdgeOffset=" << newEdgeOffset << std::endl;
#endif
    
    // set atomic int and increase
    pContext->curParPtr->mEdgeVec[startingOffset +  pContext->curParPtr->addedOffset[idx]] = xilinx_apps::louvainmod::Edge((long)louvainIdSource,(long)louvainIdTarget, (double)wtAttr);
   /*
    if(louvainIdSource < 10) {
        std::cout << "DEBUG:: louvainIdSource : " << louvainIdSource<< "; pContext->getLouvainOffset():" <<  pContext->getLouvainOffset() << std::endl;
        std::cout << "DEBUG:: idx: " << pContext->idx << std::endl;
        std::cout << "DEBUG:: starting offset: "  << pContext->mEdgePtrVec[pContext->idx]<< std::endl;


        //std::cout << "DEBUG:: startingOffset: " << pContext->startingOffset << "; idx : " << pContext->idx << std::endl;
    }*/
    pContext->curParPtr->mDgrVec[startingOffset+ pContext->curParPtr->addedOffset[idx]] = outDgr;
    pContext->curParPtr->addedOffset[idx]++;
#ifdef XILINX_COM_DETECT_DEBUG_ON
    if (pContext->curParPtr->edgeCount == 0)
        std::cout << "DEBUG: " << __FUNCTION__ << " end first edge" << std::endl;
    if (++pContext->curParPtr->edgeCount % 1000000 == 0)
        std::cout << "DEBUG: " << __FUNCTION__ << " processed " << pContext->curParPtr->edgeCount << " edges." << std::endl;
#endif
#ifdef XILINX_COM_DETECT_DUMP_MTX
    pContext->mtxFstream << louvainIdSource << ' ' << louvainIdTarget << ' ' << wtAttr << std::endl;
#endif
#endif
}

inline int udf_save_alveo_partition(uint numPar, bool isWholeGraph) {
    std::cout << "AGML-INFO: " << __FUNCTION__ << " numPar=" << numPar << " isWholeGraph=" << isWholeGraph << std::endl;
#ifndef XILINX_COM_DETECT_STUBS_ONLY

    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
    xilComDetect::Context *pContext = xilComDetect::Context::getInstance();
    //build offsets_tg
    
    //build dgr list and edgelist

    //traverse the partition size for each louvainId, populate edgelist from edgeListMap
    pContext->setStartVertex(long(pContext->getLouvainOffset()));
    pContext->setEndVertex(long(pContext->getLouvainOffset() + pContext->getNextId())) ; // the end vertex on local partition
    pContext->setDrglistTg(pContext->curParPtr->mDgrVec.data());
    pContext->setEdgelistTg(pContext->curParPtr->mEdgeVec.data());

#ifdef XILINX_COM_DETECT_DEBUG_ON
    std::cout << "    edgelist size:" << pContext->curParPtr->mEdgeVec.size() << std::endl;
    std::cout << "    dgrlist size:" << pContext->curParPtr->mDgrVec.size() << std::endl;
    for(int i=0; i < 10; i++) {
        std::cout << "        " << i << " head from edgelist  " << pContext->getEdgelistTg()[i].head <<"; ";
        std::cout << " tail from edgelist: " << pContext->getEdgelistTg()[i].tail <<"; ";
        std::cout << " outDgr from dgrlist: " << pContext->getDrglistTg()[i] << std::endl;
    }

    std::cout << "start_vertex: " << pContext->getStartVertex() <<std::endl;
    std::cout << "end_vertex: " << pContext->getEndVertex() <<std::endl;
#endif
    long numVertices = pContext->getEndVertex() - pContext->getStartVertex(); 
    long numEdges = pContext->curParPtr->mEdgeVec.size(); 
    long NV_par_requested;
    long NV_par_max = 64*1000*1000;
  
    std::cout << "AGML-INFO: Graph stats:" 
              << "\n    numVertices=" << numVertices 
              << "\n    numEdges=" << numEdges 
              << "\n    NV_par_max=" << NV_par_max << std::endl;

    if (numPar>1)
        NV_par_requested = ( pContext->getNextId()+ numPar-1) / numPar;//allow to partition small graph with -par_num
    else
        NV_par_requested = (long)((float)NV_par_max * 0.80);//20% space for ghosts.

    xilinx_apps::louvainmod::LouvainMod *pLouvainMod = pContext->getLouvainModObj();
    xilinx_apps::louvainmod::LouvainMod::PartitionData partitionData;
    
    partitionData.NV_par_max = NV_par_max;
    partitionData.offsets_tg = pContext->getOffsetsTg(); //offsets_tg;
    partitionData.edgelist_tg = pContext->getEdgelistTg(); // edgelist_tg;
    partitionData.drglist_tg = pContext->getDrglistTg(); //drglist_tg;
    partitionData.start_vertex = pContext->getStartVertex(); //start_vertex;
    partitionData.end_vertex = pContext->getEndVertex(); //end_vertex;
    partitionData.isWholeGraph = isWholeGraph;
    if (numPar > 1)
        partitionData.NV_par_requested = NV_par_requested;

    int64_t number_of_partitions = (int64_t)pLouvainMod->addPartitionData(partitionData);
    std::cout << "AGML-INFO: " << __FUNCTION__ << " final number_of_partitions=" << number_of_partitions << std::endl;
    
    return number_of_partitions;
#endif
#ifdef XILINX_COM_DETECT_DEBUG_ON
    std::cout << "DEBUG: End " << __FUNCTION__ << std::endl;
#endif
}

inline void udf_finish_partition(MapAccum<uint64_t, int64_t> numAlveoPars){
#ifdef XILINX_COM_DETECT_DEBUG_ON
    std::cout << "DEBUG: " << __FUNCTION__ << std::endl;
#endif
#ifndef XILINX_COM_DETECT_STUBS_ONLY
    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
    xilComDetect::Context *pContext = xilComDetect::Context::getInstance();

#ifdef XILINX_COM_DETECT_DUMP_MTX
    pContext->mtxFstream.close();
#endif

    xilinx_apps::louvainmod::LouvainMod *pLouvainMod = pContext->getLouvainModObj();
    pContext->clearNumAlveoPartitions();
    for(unsigned i=0;i<numAlveoPars.size();i++){
        pContext->addNumAlveoPartitions(((int)numAlveoPars.get(i)));
    }
#ifdef XILINX_COM_DETECT_DEBUG_ON
    for(unsigned i=0;i<numAlveoPars.size();i++){
        std::cout << "numAlveoPartition: " << pContext->getNumAlveoPartitions()[i] <<std::endl;
    }
#endif
    
    pLouvainMod->finishPartitioning(pContext->getNumAlveoPartitions().data());
    
    delete pContext->curParPtr;
#endif
#ifdef XILINX_COM_DETECT_DEBUG_ON
    std::cout << "DEBUG: End " << __FUNCTION__ << std::endl;
#endif
}

// TODO: Change signature as needed
// This function combined with GSQL code should traverse memory of TigerGraph on Each
// server and build the partitions for each server in the form Louvain host code can consume it
inline int udf_load_alveo(uint num_partitions, uint num_devices)
{
    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
    xilComDetect::Context *pContext = xilComDetect::Context::getInstance();
    int retVal = 0;
    retVal = load_alveo_partitions(num_partitions, num_devices);    

 /* TODO
    std::lock_guard<std::mutex> lockGuard(xai::writeMutex);
    int ret=0;
    if (!xai::loadedAlveo) {
        if(xai::loadedAlveo) return 0;
        xai::loadedAlveo = true;
        xai::num_partitions = num_partitions;
        xai::num_devices = num_devices;
    }
*/    
    // TODO: make call into host code and add needed functions
    return retVal;
}


//add for loading mtx
inline int udf_create_and_load_alveo_partitions(bool use_saved_partition,
    std::string graph_file, std::string alveo_project, uint num_nodes, uint num_partitions,
    uint gh_par)
{
    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
    xilComDetect::Context *pContext = xilComDetect::Context::getInstance();

    xilinx_apps::louvainmod::Options options;
          options.xclbinPath = PLUGIN_XCLBIN_PATH_U50;
          options.nameProj = alveo_project;
          options.numDevices = pContext->getNumDevices();
          options.deviceNames = pContext->deviceNames_;
          options.nodeId = pContext->getNodeId();
          options.hostName = pContext->curNodeHostname_;
          options.clusterIpAddresses = pContext->getCurNodeIp();
          options.hostIpAddress = pContext->getCurNodeIp();

#ifdef XILINX_COM_DETECT_DEBUG_ON
          std::cout << "DEBUG: udf_create_and_load_alveo_partitions. louvainmod options:"
                  << "\n    xclbinPath=" << options.xclbinPath
                  << "\n    nameProj=" << options.nameProj
                  << "\n    numDevices=" << options.numDevices
                  << "\n    deviceNames=" << options.deviceNames
                  << "\n    nodeId=" << options.nodeId
                  << "\n    hostName=" << options.hostName
                  << "\n    clusterIpAddresses=" << options.clusterIpAddresses
                  << "\n    hostIpAddress=" << options.hostIpAddress <<std::endl;
#endif
          xilinx_apps::louvainmod::LouvainMod* pLouvainMod = new xilinx_apps::louvainmod::LouvainMod(options);
          int ret=0;

          xilinx_apps::louvainmod::LouvainMod::PartitionOptions partOpts;

          if (!use_saved_partition && pContext->getNodeId() == 0) {
              std::cout << "DEBUG: Calling create_alveo_partitions" << std::endl;
              //ret = create_and_load_alveo_partitions(argc, (char**)(argv));
              partOpts.numPars = num_partitions;
              partOpts.par_prune = gh_par;

              pLouvainMod->partitionDataFile(&graph_file[0], partOpts);

          }
}

inline bool udf_close_alveo(int mode)
{
    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
    return true;
}

inline int udf_execute_reset(int mode) {
    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
    xilComDetect::Context *pContext = xilComDetect::Context::getInstance();
    pContext->clear();
}

inline float udf_louvain_alveo(
    const std::string& prj_pathname, const std::string& graph_name, int64_t max_iter, int64_t max_level, float tolerance, bool intermediateResult,
    bool verbose, const std::string& result_file, bool final_Q, bool all_Q)
{        
    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
    xilComDetect::Context *pContext = xilComDetect::Context::getInstance();

    //std::string alveo_parj_path = pContext->getXGraphStorePath() + "/" + graph_name;
    //std::cout << "DEBUG: in udf_louvain_alveo alveo_parj_path = " << alveo_parj_path << std::endl;
    //pContext->setAlveoProject(alveo_parj_path);
    if(prj_pathname.empty()) {
        pContext->setAlveoProject(graph_name);
        pContext->setPartitionNameMode(xilComDetect::PartitionNameMode::Auto);
    } else {
        pContext->setAlveoProjectRaw(prj_pathname);
        pContext->setPartitionNameMode(xilComDetect::PartitionNameMode::Flat);
    }
    xilinx_apps::louvainmod::LouvainMod *pLouvainMod = pContext->getLouvainModObj();

    // when restore pContext, nodeId  is missing
 //   unsigned nodeId = pContext->getNodeId();
    std::string alveoProject = pContext->getAlveoProject() + ".par.proj";
    xilComDetect::LouvainMod::ComputeOptions computeOpts;

    float finalQ;

    pLouvainMod->setAlveoProject((char *)alveoProject.c_str());

    computeOpts.outputFile = (char *)result_file.c_str();
    computeOpts.max_iter = max_iter;
    computeOpts.max_level = max_level; 
    computeOpts.tolerance = tolerance; 
    computeOpts.intermediateResult = intermediateResult;
    computeOpts.final_Q = final_Q;
    computeOpts.all_Q = all_Q; 

    std::cout
        << "DEBUG: " << __FUNCTION__ 
        << "\n    computeOpts.max_iter=" << computeOpts.max_iter 
        << "\n    computeOpts.max_level=" << computeOpts.max_level
        << "\n    computeOpts.tolerance=" << computeOpts.tolerance 
        << "\n    computeOpts.intermediateResult=" << computeOpts.intermediateResult
        << "\n    computeOpts.final_Q=" << computeOpts.final_Q 
        << "\n    computeOpts.all_Q=" << computeOpts.all_Q 
        << std::endl;
 
    finalQ = pLouvainMod->loadAlveoAndComputeLouvain(computeOpts); 
    
    std::cout << "DEBUG: Returned from " << __FUNCTION__ 
              << " finalQ=" << finalQ << std::endl;

    return finalQ;
}

// mergeHeaders 1 section body end xilinxComDetect DO NOT REMOVE!

}

#endif /* XILINX_COM_DETECT_HPP */

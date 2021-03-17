#include <vector>
#include <cstdint>
#include <stdlib.h>
#include <cstring>
#include <memory>
#include <unordered_map>
#include "cosinesim.hpp"
#include "xf_graph_L3.hpp"



namespace xilinx_apps {
namespace cosinesim {
class PrivateImpl : public ImplBase {

public:
	int indexDeviceCuNm, indexSplitNm, indexNumVertices, indexVecLength;
	const int splitNm = 3;    // kernel has 4 PUs, the input data should be splitted into 4 parts
	const int channelsPU = 4; // each PU has 4 HBM channels
	const int cuNm = 2;
	const int channelW = 16;
	const int32_t nullVal = 0;

	int valueSize_;
	int64_t devicesNeeded;
	int populationVectorRowNm;
	int32_t** numVerticesPU ; // vertex numbers in each PU


	int32_t numEdges;
	int vecLength;
	int64_t numVertices;
	int32_t edgeAlign8;
	int subChNm;
    int lastPopulationCnt;
	//std::vector<int> loadOldVectorCnt(channelsPU,0);
	std::vector<int> loadPopulationCnt;

	//xf::graph::Graph<int32_t, int32_t>** g = new xf::graph::Graph<int32_t, int32_t>*[deviceNeeded * cuNm];
	//currently only support int32_t graph
	std::vector<xf::graph::Graph<int32_t, int32_t>*> g;
	//xf::graph::Graph<int32_t, int32_t>** g;

	// initial value is 0; means no errors;
	ErrorCode errorCode_;

	//options
	Options options_;

private:
	//CosineSimBase* cosinesimPtr;

public:
	//PrivateImpl(CosineSimBase* ptr, unsigned valueSize){
	PrivateImpl(Options options, unsigned valueSize){
		errorCode_ = NoError;
		valueSize_ = valueSize;
		//cosinesimPtr = ptr;
		indexDeviceCuNm=0;
		indexSplitNm=0;
		indexNumVertices=0;
		indexVecLength=3;
		populationVectorRowNm=0;
		subChNm = 1;
		//vecLength = ptr->getOptions().vecLength;
		vecLength = options.vecLength;
		//numEdges = vecLength - 3;
		numEdges = vecLength;
		//numVertices =options.numVertices;
		devicesNeeded = options.devicesNeeded;
	    options_ = options;
		edgeAlign8 = ((numEdges + channelW - 1) / channelW) * channelW;

		numVerticesPU  = new int32_t*[devicesNeeded * cuNm];
		for (int i = 0; i < devicesNeeded * cuNm; ++i) {
		        numVerticesPU[i] = new int32_t[splitNm];
		}

	    edgeAlign8 = ((numEdges + channelW - 1) / channelW) * channelW;

		loadPopulationCnt.resize(channelsPU,0);
		g.resize(devicesNeeded*cuNm);
        // g = new xf::graph::Graph<int32_t, int32_t>*[devicesNeeded * cuNm];



	}

	~PrivateImpl(){}

   virtual void startLoadPopulation(std::int64_t numVertices){
		indexDeviceCuNm=0;
		indexSplitNm=0;
		indexNumVertices=0;
		indexVecLength=3;
		populationVectorRowNm=0;


		vecLength = options_.vecLength;
		numEdges = options_.vecLength;
		//numVertices = cosinesimPtr->getOptions().numVertices;
		this->numVertices = numVertices;
		devicesNeeded = options_.devicesNeeded;

		//the following calculation and assignment is based on numVertices
		int general = ((numVertices - (devicesNeeded * cuNm * splitNm * channelsPU + 1)) /
		                   (devicesNeeded * cuNm * splitNm * channelsPU)) * channelsPU;
		int rest = numVertices - general * (devicesNeeded * cuNm * splitNm - 1);

		std::cout << "DEBUG: " << __FILE__ << "::" << __FUNCTION__
	            << " numVertices=" << numVertices << ", general=" << general
	            << ", rest=" << rest
	            << ", split=" << devicesNeeded * cuNm * splitNm << std::endl;

		if (rest < 0) {
	        errorCode_= ErrorGraphPartition;
	    }

		for (int i = 0; i < devicesNeeded * cuNm; ++i) {
			for (int j = 0; j < splitNm; ++j) {
				numVerticesPU[i][j] = general;
			}
		}
		numVerticesPU[devicesNeeded * cuNm - 1][splitNm - 1] = rest;
		//initialize graph properties
		int fpgaNodeNm = 0;
		for(int i=0;i<devicesNeeded*cuNm; i++){
			//TODO change to Value
			//g[i] = new xf::graph::Graph<int32_t, int32_t>("Dense", 4 * splitNm, numEdges, numVerticesPU[i]);

			if(valueSize_ == 4) {
				g[i] = new xf::graph::Graph<int32_t, int32_t>("Dense", 4 * splitNm, numEdges,  numVerticesPU[i]);
			//} else if(valueSize_ == 8) {
			//	g[i] = new xf::graph::Graph<int32_t, int64_t>("Dense", 4 * splitNm, numEdges, numVerticesPU[i]);
			} else {
				errorCode_ =  ErrorUnsupportedValueType;
			}
			g[i]->numEdgesPU = new int32_t[splitNm];
			g[i]->numVerticesPU = new int32_t[splitNm];
			g[i]->edgeNum = numEdges;
			g[i]->nodeNum = numVertices;
			g[i]->splitNum = splitNm;
			g[i]->refID = fpgaNodeNm;
	        for (int j = 0; j < splitNm; ++j) {
	            fpgaNodeNm += numVerticesPU[i][j];
	            int depth = ((numVerticesPU[i][j] + channelsPU - 1) / channelsPU) * edgeAlign8;
	            g[i]->numVerticesPU[j] = numVerticesPU[i][j];
	            g[i]->numEdgesPU[j] = depth;
	            //row padding move to finishCurrentPopulationVector()
	            /*
	            for (int l = 0; l < channelsPU; ++l) {
	                for (int k = 0; k < depth; ++k) {
	                    g[i][0].weightsDense[j * channelsPU + l][k] = nullVal;
	                }
	            }*/
	        }
		}



	}
   void loadgraph_cosinesim_ss_dense_fpga(unsigned deviceNeeded,unsigned cuNm, xf::graph::Graph<int32_t, int32_t>** g) ;

   virtual void *getPopulationVectorBuffer(RowIndex &rowIndex){

	   void * pbuf;
	   if(indexDeviceCuNm == devicesNeeded * cuNm)
		   return nullptr;
	   subChNm = (numVerticesPU[indexDeviceCuNm][indexSplitNm] + channelsPU - 1) / channelsPU;

	   pbuf = &(g[indexDeviceCuNm]->weightsDense[indexSplitNm * channelsPU + indexNumVertices / subChNm][loadPopulationCnt[indexNumVertices / subChNm] * edgeAlign8]);

	   loadPopulationCnt[indexNumVertices / subChNm] += 1;
	   lastPopulationCnt = loadPopulationCnt[indexNumVertices / subChNm];
	   indexNumVertices++;
	   if(indexNumVertices ==numVerticesPU[indexDeviceCuNm][indexSplitNm] ) {
		   indexSplitNm++;
		   indexNumVertices=0;
		   std::fill(loadPopulationCnt.begin(),loadPopulationCnt.end(),0);
		   if(indexSplitNm == splitNm) {
			   indexDeviceCuNm++;
			   indexSplitNm = 0;
		   }
	   }

	   rowIndex = populationVectorRowNm++;
	   return pbuf;

	}

	virtual void finishCurrentPopulationVector(void * pbuf){
		memset(pbuf+vecLength*valueSize_, 0, ( edgeAlign8 - vecLength)*valueSize_ );


	}

	//padding the row and loadgraph
	virtual void finishLoadPopulationVectors() {

		//for the last PU, the last channels may need to padded as the row will be the multiple of channelsPU
		int paddingDepth = ((numVerticesPU[indexDeviceCuNm-1][indexSplitNm-1] + channelsPU - 1) / channelsPU)*channelsPU - numVerticesPU[indexDeviceCuNm-1][indexSplitNm-1] ;
		int startPaddingDepth = numVerticesPU[indexDeviceCuNm-1][indexSplitNm-1] * edgeAlign8;
		for (int k = startPaddingDepth; k < paddingDepth*edgeAlign8; ++k) {
			g[indexDeviceCuNm]->weightsDense[ (indexSplitNm-1) * channelsPU + 3][k] = nullVal;
		}
/*
		unsigned depth = g[cuNm - 1]->numEdgesPU[splitNm - 1];  // reserved space for each channel of last PU
		unsigned totalPURows = depth / edgeAlign8 * channelsPU;  // reserved rows for all 4 channels of last PU
		unsigned lastPUNumVectors = numVerticesPU[devicesNeeded * cuNm - 1][splitNm - 1];  // # vecs in last PU
		unsigned numPadRows =  totalPURows - lastPUNumVectors;  // how many rows need to be padded
		for (unsigned chan = channelsPU - numPadRows; chan < channelsPU; ++chan)
		    for (unsigned k = 0; k < edgeAlign8; ++k)
		        g[cuNm - 1]->weightsDense[(splitNm - 1) * chan][depth - edgeAlign8 + k] = 0;
*/
		loadgraph_cosinesim_ss_dense_fpga(devicesNeeded, cuNm, g.data());

	}


	void cosinesim_ss_dense_fpga(int32_t deviceNeeded,int32_t sourceLen,int32_t* sourceWeight,
	        int32_t topK,xf::graph::Graph<int32_t, int32_t>** g,int32_t* resultID,
	        float* similarity) ;

	int cosineSimilaritySSDenseMultiCard(std::shared_ptr<xf::graph::L3::Handle>& handle,
	                                     int32_t deviceNm,
	                                     int32_t sourceNUM,
	                                     int32_t* sourceWeights,
	                                     int32_t topK,
	                                     xf::graph::Graph<int32_t, int32_t>** g,
	                                     int32_t* resultID,
	                                     float* similarity);

	std::vector<xf::graph::L3::event<int> > cosineSimilaritySSDenseMultiCard(std::shared_ptr<xf::graph::L3::Handle>& handle,
	                                                          int32_t deviceNm,
	                                                          int32_t sourceNUM,
	                                                          int32_t* sourceWeights,
	                                                          int32_t topK,
	                                                          xf::graph::Graph<int32_t, int32_t>** g,
	                                                          int32_t** resultID,
	                                                          float** similarity);



	virtual std::vector<Result> matchTargetVector(unsigned numResults, void *elements){

		std::vector<Result> result;
	    //---------------- Generate Source Indice and Weight Array -------
	    int sourceLen = edgeAlign8; // sourceIndice array length
	    int32_t* sourceWeight =
	        xf::graph::internal::aligned_alloc<int32_t>(sourceLen); // weights of source vertex's out members
	    //int32_t newVecLen = newVector.size() - 3;

	    for (int i = 0; i < sourceLen; i++) {
	        if (i < vecLength) {
	        	//sourceWeight[i] = elements[i + 3];

	        	if(valueSize_ == 4)
	        		sourceWeight[i] = (reinterpret_cast<int32_t*>(elements))[i];
	        	//else if (valueSize_== 8)
	        	//	sourceWeight[i] = (reinterpret_cast<int64_t*>(elements))[i];
	        	else {
	        		errorCode_ =  ErrorUnsupportedValueType;
	        	}

	        } else {
	            sourceWeight[i] = nullVal;
	        }
	    }
	    float* similarity = xf::graph::internal::aligned_alloc<float>(numResults);
	    int32_t* resultID = xf::graph::internal::aligned_alloc<int32_t>(numResults);
	    std::memset(resultID, 0, numResults * sizeof(int32_t));
	    std::memset(similarity, 0, numResults * sizeof(float));

	    //---------------- Run L3 API -----------------------------------

	    cosinesim_ss_dense_fpga(devicesNeeded * cuNm, sourceLen, sourceWeight, numResults, g.data(),resultID, similarity);

	    for (unsigned int k = 0; k < numResults; k++) {
	        //result += testResults(VERTEX(xai::IDMap[resultID[k]]), similarity[k]);
	    	result.push_back(Result(resultID[k], similarity[k]));
	    }

	    //--------------- Free and delete -----------------------------------

	    for (int i = 0; i < devicesNeeded * cuNm; ++i) {
	    	g[i]->freeBuffers();
	    	delete[] g[i]->numEdgesPU;
	    	delete[] g[i]->numVerticesPU;
	    }
	    //delete[] g;


	    return result;
	}

}; // class PrivateImpl

class sharedHandlesCosSimDense {
   public:
    std::unordered_map<unsigned int, std::shared_ptr<xf::graph::L3::Handle> > handlesMap;
    static sharedHandlesCosSimDense& instance() {
        static sharedHandlesCosSimDense theInstance;
        return theInstance;
    }
};

int PrivateImpl::cosineSimilaritySSDenseMultiCard(std::shared_ptr<xf::graph::L3::Handle>& handle,
                                     int32_t deviceNm,
                                     int32_t sourceNUM,
                                     int32_t* sourceWeights,
                                     int32_t topK,
                                     xf::graph::Graph<int32_t, int32_t>** g,
                                     int32_t* resultID,
                                     float* similarity) {
    std::vector<xf::graph::L3::event<int> > eventQueue;
    float** similarity0 = new float*[deviceNm];
    int32_t** resultID0 = new int32_t*[deviceNm];
    int counter[deviceNm];
    for (int i = 0; i < deviceNm; ++i) {
        counter[i] = 0;
        similarity0[i] = xf::graph::internal::aligned_alloc<float>(topK);
        resultID0[i] = xf::graph::internal::aligned_alloc<int32_t>(topK);
        memset(resultID0[i], 0, topK * sizeof(int32_t));
        memset(similarity0[i], 0, topK * sizeof(float));
    }
    for (int i = 0; i < deviceNm; ++i) {
        eventQueue.push_back((handle->opsimdense)->addworkInt(
            1, 0, sourceNUM, sourceWeights, topK, g[i][0], resultID0[i], similarity0[i]));
    }
    int ret = 0;
    for (int i = 0; i < eventQueue.size(); ++i) {
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

std::vector<xf::graph::L3::event<int> > PrivateImpl::cosineSimilaritySSDenseMultiCard(std::shared_ptr<xf::graph::L3::Handle>& handle,
                                                          int32_t deviceNm,
                                                          int32_t sourceNUM,
                                                          int32_t* sourceWeights,
                                                          int32_t topK,
                                                          xf::graph::Graph<int32_t, int32_t>** g,
                                                          int32_t** resultID,
                                                          float** similarity) {
    std::vector<xf::graph::L3::event<int> > eventQueue;

    for (int i = 0; i < deviceNm; ++i) {
        eventQueue.push_back(
            (handle->opsimdense)
                ->addworkInt(1, 0, sourceNUM, sourceWeights, topK, g[i][0], resultID[i], similarity[i]));
    }
    return eventQueue;
};

void PrivateImpl::cosinesim_ss_dense_fpga(int32_t devicesNeeded,
                                        int32_t sourceLen,
                                        int32_t* sourceWeight,
                                        int32_t topK,
                                        xf::graph::Graph<int32_t, int32_t>** g,
                                        int32_t* resultID,
                                        float* similarity) {
    //---------------- Run Load Graph -----------------------------------
    //std::cout << "DEBUG: " << __FILE__ << "::" << __FUNCTION__
    //          << " XRT_INI_PATH=" << std::getenv("XRT_INI_PATH") << std::endl;

    std::chrono::time_point<std::chrono::high_resolution_clock> l_start_time =
            std::chrono::high_resolution_clock::now();
    std::cout << "DEBUG: " << __FUNCTION__ << " start=" << l_start_time.time_since_epoch().count()
              << std::endl;

    std::shared_ptr<xf::graph::L3::Handle> handle0 =
                        sharedHandlesCosSimDense::instance().handlesMap[0];
    handle0->debug();
    int32_t requestNm = 1;
    int32_t hwNm = devicesNeeded;
    std::cout << "DEBUG: " << __FILE__ << "::" << __FUNCTION__
              << " hwNm=" << hwNm << std::endl;
    std::vector<xf::graph::L3::event<int> > eventQueue[requestNm];
    float** similarity0[requestNm];
    int32_t** resultID0[requestNm];
    int counter[requestNm][hwNm];
    for (int m = 0; m < requestNm; ++m) {
        similarity0[m] = new float*[hwNm];
        resultID0[m] = new int32_t*[hwNm];
        for (int i = 0; i < hwNm; ++i) {
            counter[m][i] = 0;
            similarity0[m][i] = xf::graph::internal::aligned_alloc<float>(topK);
            resultID0[m][i] = xf::graph::internal::aligned_alloc<int32_t>(topK);
            memset(resultID0[m][i], 0, topK * sizeof(int32_t));
            memset(similarity0[m][i], 0, topK * sizeof(float));
        }
    }
    //---------------- Run L3 API -----------------------------------
    std::cout << "DEBUG: " << __FILE__ << "::" << __FUNCTION__
             << " sourceLen=" << sourceLen << ", topK=" << topK
             << ", sourceWeight=" ;
     for(int i=0;i<sourceLen; i++){
     	std::cout <<i<< " : " << sourceWeight[i]<<std::endl;
     }
    for (int m = 0; m < requestNm; ++m) {
        eventQueue[m] = cosineSimilaritySSDenseMultiCard(
            handle0, hwNm, sourceLen, sourceWeight, topK, g,
            resultID0[m], similarity0[m]);
    }

    int ret = 0;
    for (int m = 0; m < requestNm; ++m) {
        for (int i = 0; i < eventQueue[m].size(); ++i) {
            ret += eventQueue[m][i].wait();
        }
    }
    for (int m = 0; m < requestNm; ++m) {
        for (int i = 0; i < topK; ++i) {
            similarity[i] = similarity0[m][0][counter[m][0]];
            int32_t prev = 0;
            resultID[i] = resultID0[m][0][counter[m][0]];
            counter[m][0]++;
            for (int j = 1; j < hwNm; ++j) {
                if (similarity[i] < similarity0[m][j][counter[m][j]]) {
                    similarity[i] = similarity0[m][j][counter[m][j]];
                    resultID[i] = resultID0[m][j][counter[m][j]];
                    counter[m][prev]--;
                    prev = j;
                    counter[m][j]++;
                }
            }
        }
    }

    for (int m = 0; m < requestNm; ++m) {
        for (int i = 0; i < hwNm; ++i) {
            free(similarity0[m][i]);
            free(resultID0[m][i]);
        }
        delete[] similarity0[m];
        delete[] resultID0[m];
    }
    std::chrono::time_point<std::chrono::high_resolution_clock> l_end_time =
            std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> l_durationSec = l_end_time - l_start_time;
    double l_timeMs = l_durationSec.count() * 1e3;
    std::cout << "PROFILING: " << __FILE__ << "::" << __FUNCTION__
              << " runtime msec=  " << std::fixed << std::setprecision(6)
              << l_timeMs << std::endl;
}

void PrivateImpl::loadgraph_cosinesim_ss_dense_fpga(unsigned deviceNeeded,
                                                  unsigned cuNm,
                                                  xf::graph::Graph<int32_t, int32_t>** g) {
    //----------------- Text Parser --------------------------
    std::string opName;
    std::string kernelName;
    int requestLoad;
    std::string xclbinPath;


    std::string jsonFilePath = "./config_cosinesim_ss_dense_fpga.json";
    std::fstream userInput(jsonFilePath, std::ios::in);
    if (!userInput) {
        std::cout << "Error : config file " << jsonFilePath << " doesn't exist !" << std::endl;
        errorCode_ = ErrorConfigFileNotExist;
        return;
    }
    char line[1024] = {0};
    char* token;
    while (userInput.getline(line, sizeof(line))) {
        token = strtok(line, "\"\t ,}:{\n");
        while (token != NULL) {
            if (!std::strcmp(token, "operationName")) {
                token = strtok(NULL, "\"\t ,}:{\n");
                opName = token;
            } else if (!std::strcmp(token, "kernelName")) {
                token = strtok(NULL, "\"\t ,}:{\n");
                kernelName = token;
            } else if (!std::strcmp(token, "requestLoad")) {
                token = strtok(NULL, "\"\t ,}:{\n");
                requestLoad = std::atoi(token);
            } else if (!std::strcmp(token, "xclbinPath")) {
                token = strtok(NULL, "\"\t ,}:{\n");
                std::string tmpStr = token;
                xclbinPath = tmpStr;
            }
            token = strtok(NULL, "\"\t ,}:{\n");
        }
    }
    userInput.close();

    //----------------- Setup denseSimilarityKernel thread ---------
    xf::graph::L3::Handle::singleOP op0;
    op0.operationName = (char*)opName.c_str();
    op0.setKernelName((char*)kernelName.c_str());
    op0.requestLoad = requestLoad;
    op0.xclbinFile = (char*)xclbinPath.c_str();
    op0.deviceNeeded = deviceNeeded;
    op0.cuPerBoard = cuNm;

    std::fstream xclbinFS(xclbinPath, std::ios::in);
    if (!xclbinFS) {
        std::cout << "Error : xclbinFile doesn't exist: " << xclbinPath << std::endl;
        errorCode_ = ErrorXclbinNotExist;
        return;
    }

    std::shared_ptr<xf::graph::L3::Handle> handleInstance(new xf::graph::L3::Handle);
    sharedHandlesCosSimDense::instance().handlesMap[0] = handleInstance;
    std::shared_ptr<xf::graph::L3::Handle> handle0 = sharedHandlesCosSimDense::instance().handlesMap[0];

    handle0->addOp(op0);
    int status = handle0->setUp();
    if (status < 0) {
    	errorCode_ = ErrorFailFPGASetup;
    	return;
    }

    //-------------------------
    std::shared_ptr<xf::graph::L3::Handle> handle1 = sharedHandlesCosSimDense::instance().handlesMap[0];
    handle1->debug();
    //------------------------

    //---------------- Run Load Graph -----------------------------------
    for (int i = 0; i < deviceNeeded * cuNm; ++i) {
        std::cout << "DEBUG: loadGraphMultiCardNonBlocking " << i << std::endl;
        (handle0->opsimdense)->loadGraphMultiCardNonBlocking(i / cuNm, i % cuNm, g[i][0]);
    }

}

extern "C" {
    ImplBase *createImpl(const Options& options, unsigned valueSize){
    	return new PrivateImpl(options,valueSize);
    }

    void destroyImpl(ImplBase *pImpl){
    	delete pImpl;
    }
}


} //cosinesim
} //xilinx_app

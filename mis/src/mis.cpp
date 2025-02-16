/*
 * Copyright 2019-2021 Xilinx, Inc.
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
#include <iostream>
#include <sstream>

#include <xrt/xrt_kernel.h>
#include <xrt/xrt_bo.h>
#include "xilinxmis.hpp"
#include "utils.hpp"

namespace xilinx_apps {
namespace mis {

template <typename T>
struct aligned_allocator {
    using value_type = T;

    aligned_allocator() {}

    aligned_allocator(const aligned_allocator&) {}

    template <typename U>
    aligned_allocator(const aligned_allocator<U>&) {}

    T* allocate(std::size_t num) {
        void* ptr = nullptr;
#if defined(_WINDOWS)
        ptr = _aligned_malloc(num * sizeof(T), 4096);
        if (ptr == nullptr) {
            std::cout << "Failed to allocate memory" << std::endl;
            exit(EXIT_FAILURE);
        }
#else
        if (posix_memalign(&ptr, 4096, num * sizeof(T))) {
            std::cout << "Failed to allocate memory." << std::endl;
            throw std::bad_alloc();
        }
#endif
        return reinterpret_cast<T*>(ptr);
    }
    void deallocate(T* p, std::size_t num) {
#if defined(_WINDOWS)
        _aligned_free(p);
#else
        free(p);
#endif
    }
};

class MisImpl : public ImplBase {
   public:
    Options options_;
    MisImpl(const Options& options) : options_(options) {}
    virtual ~MisImpl() {}

    static const int mVertexLimits = 1024000;
    int device_id = 0;

    size_t mMemSize;
    int mNumChannels;
    int mEntrySize;
    bool mHBM;

    xrt::device mDevice;
    xrt::kernel mKernel;
    xrt::bo d_rowPtr;
    xrt::bo d_mPrior;

    uint16_t* mPrior;
    const GraphCSR* mOrigGraph;  // not owned
    std::vector<int, aligned_allocator<int> > mRowPtr;
    std::vector<int*> mColIdx;
    std::vector<xrt::bo> d_sync, d_colIdx;
    // The initialization process will download FPGA binary to FPGA card
    void startMis(const std::string& xclbinPath, const std::string& deviceNames);
    void graphPadding();
    void evict(const std::vector<int>&);
    void fresh();
    void genPrior();
    bool verifyMis();

    int getDevice(const std::string& deviceNames);

    // internal only
    // static void init(const GraphCSR<host_buffer_t<int> >*, host_buffer_t<uint16_t>&);
    // static bool verifyMis(GraphCSR<host_buffer_t<int> >*, const host_buffer_t<uint8_t>&);
    // static size_t count(const int, const host_buffer_t<uint8_t>&);

    // ImplBase Implementation
    
    virtual void startMis() override { startMis(options_.xclbinPath, options_.deviceNames); }
    virtual void setGraph(const GraphCSR* graph) override;
    virtual XVector<XVector<int>> executeMIS(int iter) override;
    virtual size_t count() const override;
};

int MisImpl::getDevice(const std::string& deviceNames) {
    int status = -1; // initialize to no match device found
    int count = xclProbe();

    for (int i = 0; i < count; ++i) {
        mDevice = xrt::device(i);
        std::string curDeviceName = mDevice.get_info<xrt::info::device::name>();

        // the device name on aws-f1 may show up in three forms
        if ((deviceNames == "u50" && curDeviceName == "xilinx_u50_gen3x16_xdma_201920_3") ||
            (deviceNames == "u55c" && curDeviceName == "xilinx_u55c_gen3x16_xdma_base_2") ||
            (deviceNames == "aws-f1" && (curDeviceName == "xilinx_aws-vu9p-f1_dynamic-shell" ||
                                         curDeviceName == "xilinx_aws-vu9p-f1_shell-v04261818_201920_2" ||
                                         curDeviceName == "xilinx_aws-vu9p-f1_shell-v04261818_201920_3"))) {
            std::cout << "INFO: Found requested device: " << curDeviceName << " ID=" << i << std::endl;
            device_id = i;
            status = 0; // found a matching device
            break;
        } else {
            std::cout << "INFO: Skipped non-requested device: " << curDeviceName << " ID=" << i << std::endl;
        }
    }
    return status;
}

void MisImpl::fresh() {
    for (int i = 0; i < mOrigGraph->n; i++) {
        int st = mPrior[i] >> 14;
        if (st == 1 || mPrior[i] == 0xc000)
            mPrior[i] = 0xc000;
        else
            mPrior[i] = mPrior[i] & 0x3fff;
    }
}

void MisImpl::evict(const std::vector<int>& list) {
    for (int i = 0; i < mOrigGraph->n; i++) {
        mPrior[i] = mPrior[i] & 0x3fff;
    }
    for (const int& v : list) {
        mPrior[v] = 0xc000;
    }
}

void MisImpl::startMis(const std::string& xclbinPath, const std::string& deviceNames) {
    if (getDevice(deviceNames) < 0) {
        // std::cout << "ERROR: Unable to find device " << deviceNames << std::endl;
        // return -2;
        std::ostringstream oss;
        oss << "Unable to find device " << deviceNames << "; Please ensure the machine has proper card" << std::endl;
        throw xilinx_apps::mis::Exception(oss.str());
        std::cerr << "ERROR: Unable to find device " << deviceNames << std::endl;
        abort();
    } else
        std::cout << "INFO: Start MIS on " << deviceNames << std::endl;
    
    try {
        mDevice = xrt::device(device_id);
        auto uuid = mDevice.load_xclbin(xclbinPath);
        std::string kernelName = "misKernel:{misKernel_0}";
        mKernel = xrt::kernel(mDevice, uuid, kernelName);
        mHBM = false;
        std::cout << "INFO: Setting up parameters for Alveo with HBM memory..." << std::endl;

        if (deviceNames.find("u50") != std::string::npos) {
            mMemSize = 0x10000000;
            mNumChannels = 16;
            mEntrySize = 1;
            mHBM = true;
        } else if (deviceNames.find("u55c") != std::string::npos) {
            mMemSize = 0x20000000;
            mNumChannels = 16;
            mEntrySize = 2;
            mHBM = true;
        } else {
            if (deviceNames.find("u200") == std::string::npos && deviceNames.find("u250") == std::string::npos &&
                deviceNames.find("aws-f1") == std::string::npos) {
                std::cerr << "ERROR: Device " << deviceNames << " is not supported." << std::endl;
                abort();
            }
            mMemSize = 0x80000000;
            mNumChannels = 1;
            mEntrySize = 16;
        }

        mColIdx.resize(mNumChannels);
        d_colIdx.resize(mNumChannels);
        d_sync.resize(mNumChannels);
        for (int i = 0; i < mNumChannels; i++) {
            d_colIdx[i] = xrt::bo(mDevice, mMemSize, mKernel.group_id(i + 2));
            mColIdx[i] = d_colIdx[i].map<int*>();
        }
    } catch (std::bad_alloc& e) {
        std::cout << "Error: memory allocation failed. Maybe the CU on the device is busy." << std::endl;
//        exit(EXIT_FAILURE);
        throw;
    } catch (std::ios_base::failure& e) {
    // The following line didn't compile on AWS (CentOS 7.9) with scl devtoolset-9 because of code().
    // Needs investigation.  For now, commenting out code().
//        std::cout << "Error: I/O error during startMis, error code = " << e.code() << ". " << e.what() << std::endl;
        std::cout << "Error: I/O error during startMis. " << e.what() << std::endl;
        throw;
    } catch (const std::exception& exc) {
        std::cout << "Error: Failure during startMis: " << exc.what() << std::endl;
//        exit(EXIT_FAILURE);
        throw;
    }
    // return 0;
}

void MisImpl::genPrior() {
    constexpr int LargeNum = 65535;
    int n = mOrigGraph->n;
    // int aveDegree = graph->colIdx.size() / n;
    int aveDegree = mOrigGraph->colIdxSize / n;
    for (int i = 0; i < n; i++) {
        int degree = mOrigGraph->rowPtr[i + 1] - mOrigGraph->rowPtr[i];
        double r = (rand() % LargeNum) / (double)LargeNum;
        mPrior[i] = (aveDegree / (aveDegree + degree + r) * 8191);
        mPrior[i] &= 0x03fff;
    }
}

size_t MisImpl::count() const {
    size_t num = 0;
    for (int i = 0; i < mOrigGraph->n; i++) {
        int rp = mPrior[i] >> 14;
        if (rp == 1) num++;
    }
    return num;
}

void MisImpl::graphPadding() {
    int n = mOrigGraph->n;
    mRowPtr.resize(n + 1);
    mRowPtr[0] = 0;
    size_t maxSize = 0;
    if (mHBM) {
        std::vector<size_t> index(mNumChannels, 0);
        for (int r = 0; r < n; r++) {
            int start = mOrigGraph->rowPtr[r], stop = mOrigGraph->rowPtr[r + 1];
            for (int c = start; c < stop; c++) {
                int colId = mOrigGraph->colIdx[c];
                int ch = colId & (mNumChannels - 1);
                if (index[ch] == mMemSize / sizeof(int)) {
                    std::cout << "Graph is not supported due to memory limit." << std::endl;
                    exit(EXIT_FAILURE);
                }
                mColIdx[ch][index[ch]++] = colId;
            }

            for (int pe = 0; pe < mNumChannels; pe++)
                if (index[pe] > maxSize) maxSize = index[pe];
            maxSize += maxSize % mEntrySize;
            for (int pe = 0; pe < mNumChannels; pe++) {
                int iter = maxSize - index[pe];
                for (int i = 0; i < iter; i++) {
                    mColIdx[pe][index[pe]++] = -1;
                }
            }
            mRowPtr[r + 1] = maxSize * mNumChannels;
        }
    } else {
        std::vector<size_t> index(mEntrySize, 0);
        for (int r = 0; r < n; r++) {
            int start = mOrigGraph->rowPtr[r], stop = mOrigGraph->rowPtr[r + 1];
            for (int c = start; c < stop; c++) {
                int colId = mOrigGraph->colIdx[c];
                int ch = colId & (mEntrySize - 1);
                if (index[ch] == mMemSize / mEntrySize / sizeof(int)) {
                    std::cout << "Graph is not supported due to memory limit." << std::endl;
                    exit(EXIT_FAILURE);
                }
                mColIdx[0][index[ch] * mEntrySize + ch] = colId;
                index[ch]++;
            }

            for (int pe = 0; pe < mEntrySize; pe++)
                if (index[pe] > maxSize) maxSize = index[pe];
            for (int pe = 0; pe < mEntrySize; pe++) {
                int iter = maxSize - index[pe];
                for (int i = 0; i < iter; i++) {
                    mColIdx[0][index[pe] * mEntrySize + pe] = -1;
                    index[pe]++;
                }
            }
            mRowPtr[r + 1] = maxSize * mEntrySize;
        }
    }
}

// void MisImpl::setGraph(GraphCSR<std::vector<int> >* graph) {
void MisImpl::setGraph(const GraphCSR* graph) {
    mOrigGraph = graph;
    int n = mOrigGraph->n;
    if (n > mVertexLimits) {
        std::ostringstream oss;
        oss << "ERROR: Graph with more than " << mVertexLimits << " vertices is not supported.";
        std::cout << oss.str() << std::endl;
        //exit(EXIT_FAILURE);
        throw Exception(oss.str());
    }

    int nz = mOrigGraph->colIdxSize;

    // process the graph
    std::cout << "INFO: Processing the graph with " << n << " vertices and " << nz / 2 << " edges." << std::endl;

    // convert back tp graphCSR
    // std::unique_ptr<GraphCSR>  graphAfter{createFromGraph(graphAdj.get())};
    // auto start = chrono::high_resolution_clock::now();
    // generate h_prior
    graphPadding();

    int memSize = mRowPtr[n] / mNumChannels;
    for (int i = 0; i < mNumChannels; i++) d_sync[i] = xrt::bo(d_colIdx[i], memSize * sizeof(int), 0);
    for (auto& bo : d_sync) bo.sync(XCL_BO_SYNC_BO_TO_DEVICE);
    d_mPrior = xrt::bo(mDevice, n * sizeof(uint16_t), mKernel.group_id(2 + mNumChannels));

    mPrior = d_mPrior.map<uint16_t*>();
    d_rowPtr = xrt::bo(mDevice, (void*)mRowPtr.data(), mRowPtr.size() * sizeof(int), mKernel.group_id(1));
    d_rowPtr.sync(XCL_BO_SYNC_BO_TO_DEVICE);
}

xilinx_apps::XVector<xilinx_apps::XVector<int> > MisImpl::executeMIS(int iter) {
    std::cout << "INFO: Xilinx MIS execution..." << std::endl;
    auto run = xrt::run(mKernel);
    int nargs = 0;
    run.set_arg(nargs++, mOrigGraph->n);
    run.set_arg(nargs++, d_rowPtr);
    for (xrt::bo& bo : d_sync) run.set_arg(nargs++, bo);
    run.set_arg(nargs++, d_mPrior);

    // generate the priority values
    genPrior();

    int size = 0;
    int n = mOrigGraph->n;
    iter = iter <= 0 || iter > n ? n : iter;
    XVector<XVector<int> > ret;
    ret.reserve(iter);

    for (int i = 0; i < iter && size < n; i++) {
        fresh();
        d_mPrior.sync(XCL_BO_SYNC_BO_TO_DEVICE);
        run.start();
        run.wait();
        d_mPrior.sync(XCL_BO_SYNC_BO_FROM_DEVICE);

        XVector<int> result;
        result.reserve(n);
        for (int idx = 0; idx < n; idx++) {
            int rp = mPrior[idx] >> 14;
            if (rp == 1 && mPrior[idx] != 0xc000) result.push_back(idx);
        }
        ret.push_back(result);
        size += result.size();
    }
    return ret;
}

bool MisImpl::verifyMis() {
    int n = mOrigGraph->n;
    for (int i = 0; i < n; i++) {
        int rp = mPrior[i] >> 14;
        if (rp == 3) {
            bool oppo = false;
            for (int j = mOrigGraph->rowPtr[i]; j < mOrigGraph->rowPtr[i + 1]; j++) {
                int v = mOrigGraph->colIdx[j];
                if (v == i || v >= n) continue;
                int vp = mPrior[v] >> 14;
                oppo = oppo || (vp == 1);
            }
            if (!oppo) return false;
        } else if (rp == 1)
            for (int j = mOrigGraph->rowPtr[i]; j < mOrigGraph->rowPtr[i + 1]; j++) {
                int v = mOrigGraph->colIdx[j];
                if (v == i || v >= n) continue;
                int vp = mPrior[v] >> 14;
                if (vp == 1) {
                    std::cout << "Conflicted vertices in the set with id " << i << '\t' << v << std::endl;
                    return false;
                }
            }
        else {
            std::cout << "Undetermined vertex with id= " << i << std::endl;
            return false;
        }
    }
    return true;
}

} // namespace mis
} // namespace xilinx_apps

//#####################################################################################################################

//
// Shared Library Entry Points
//

extern "C" {

xilinx_apps::mis::ImplBase* xilinx_mis_createImpl(const xilinx_apps::mis::Options& options) {
    return new xilinx_apps::mis::MisImpl(options);
}

void xilinx_mis_destroyImpl(xilinx_apps::mis::ImplBase* pImpl) {
    delete pImpl;
}

} // extern "C"

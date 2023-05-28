This directory contains HLS source code for kernels in AMD Graph and Machine Learning (AGML) libraries. The AGML installation package includes pre-built XCLBINs for all these kernels. 
If you want to customize the kernels, follow the instructions below to rebuild XCLBINs. 

# Set up Vitis

The kernels are validated with Vitis 2022.1. Run the command below to set up Vitis environments:
```
source <install path>/Vitis/2022.1/settings64.sh
source /opt/xilinx/xrt/setup.sh
```

# Build and Test Kernels
To run a test for a specific kernel, execute the following command:
```
cd <KERNEL-NAME>
git submodule update --init --recursive
export PLATFORM_REPO_PATHS=/opt/xilinx/platforms
make run TARGET=sw_emu DEVICE=xilinx_u50_gen3x16_xdma_201920_3
```
`TARGET` can also be `hw_emu` or `hw`.

Follow the instructions below to build XCLBINs for currently supported devices. 

## Louvain Modularity Kernel
### U55C 
```

```
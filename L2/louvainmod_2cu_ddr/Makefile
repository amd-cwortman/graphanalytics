#
# Copyright 2019-2020 Xilinx, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# makefile-generator v1.0.3
#

# ####################################### Help Section #####################################
.PHONY: help

help::
	$(ECHO) "Makefile Usage:"
	$(ECHO) "  make all TARGET=<sw_emu/hw_emu/hw> DEVICE=<FPGA platform> HOST_ARCH=<aarch32/aarch64/x86>"
	$(ECHO) "      Command to generate the design for specified Target and Shell."
	$(ECHO) "      By default, HOST_ARCH=x86. HOST_ARCH is required for SoC shells"
	$(ECHO) ""
	$(ECHO) "  make clean "
	$(ECHO) "      Command to remove the generated non-hardware files."
	$(ECHO) ""
	$(ECHO) "  make cleanall"
	$(ECHO) "      Command to remove all the generated files."
	$(ECHO) ""
	$(ECHO) "  make TARGET=<sw_emu/hw_emu/hw> DEVICE=<FPGA platform> HOST_ARCH=<aarch32/aarch64/x86>"
	$(ECHO) "      By default, HOST_ARCH=x86. HOST_ARCH is required for SoC shells"
	$(ECHO) ""
	$(ECHO) "  make run TARGET=<sw_emu/hw_emu/hw> DEVICE=<FPGA platform> HOST_ARCH=<aarch32/aarch64/x86>"
	$(ECHO) "      Command to run application in emulation."
	$(ECHO) "      By default, HOST_ARCH=x86. HOST_ARCH required for SoC shells"
	$(ECHO) ""
	$(ECHO) "  make build TARGET=<sw_emu/hw_emu/hw> DEVICE=<FPGA platform> HOST_ARCH=<aarch32/aarch64/x86>"
	$(ECHO) "      Command to build xclbin application."
	$(ECHO) "      By default, HOST_ARCH=x86. HOST_ARCH is required for SoC shells"
	$(ECHO) ""
	$(ECHO) "  make host HOST_ARCH=<aarch32/aarch64/x86>"
	$(ECHO) "      Command to build host application."
	$(ECHO) "      By default, HOST_ARCH=x86. HOST_ARCH is required for SoC shells"
	$(ECHO) ""
	$(ECHO) "  NOTE: For SoC shells, ENV variable SYSROOT needs to be set."
	$(ECHO) ""

# ##################### Setting up default value of TARGET ##########################
TARGET ?= sw_emu

# ################### Setting up default value of DEVICE ##############################
#DEVICE ?= xilinx_u55c_gen3x16_xdma_2_202110_1
#DEVICE ?= xilinx_u200_gen3x16_xdma_1_202110_1
DEVICE ?= xilinx_u200_gen3x16_xdma_2_202110_1
#DEVICE ?= xilinx_aws-vu9p-f1_shell-v04261818_201920_2

# ###################### Setting up default value of HOST_ARCH ####################### 
HOST_ARCH ?= x86

# #################### Checking if DEVICE in blacklist #############################
ifeq ($(findstring zc, $(DEVICE)), zc)
$(error [ERROR]: This project is not supported for $(DEVICE).)
endif

# #################### Checking if DEVICE in whitelist ############################
#ifneq ($(findstring u55c, $(DEVICE)), u55c)
#$(warning [WARNING]: This project has not been tested for $(DEVICE). It may or may not work.)
#endif

# ######################## Setting up Project Variables #################################
MK_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
XF_PROJ_ROOT ?= $(shell bash -c 'export MK_PATH=$(MK_PATH); echo $${MK_PATH%/L2/*}')
CUR_DIR := $(patsubst %/,%,$(dir $(MK_PATH)))
XFLIB_DIR = $(XF_PROJ_ROOT)

# ######################### Include environment variables in utils.mk ####################
include ./utils.mk
XDEVICE := $(call device2xsa, $(DEVICE))
TEMP_DIR := _x_temp.$(TARGET).$(XDEVICE)
TEMP_REPORT_DIR := $(CUR_DIR)/reports/_x.$(TARGET).$(XDEVICE)
BUILD_DIR := build_dir.$(TARGET).$(XDEVICE)
BUILD_REPORT_DIR := $(CUR_DIR)/reports/_build.$(TARGET).$(XDEVICE)
EMCONFIG_DIR := $(BUILD_DIR)
XCLBIN_DIR := $(CUR_DIR)/$(BUILD_DIR)
export XCL_BINDIR = $(XCLBIN_DIR)

# ######################### Setting up Host Variables #########################
#Include Required Host Source Files
HOST_SRCS += $(XFLIB_DIR)/louvainmod/grappolo/src/driverForGraphClustering.cpp
HOST_SRCS += $(XFLIB_DIR)/louvainmod/grappolo/src/RngStream.cpp
HOST_SRCS += $(XFLIB_DIR)/louvainmod/grappolo/src/utilityFunctions.cpp
HOST_SRCS += $(XFLIB_DIR)/louvainmod/grappolo/src/parseInputFiles.cpp
HOST_SRCS += $(XFLIB_DIR)/louvainmod/grappolo/src/buildNextPhase.cpp
HOST_SRCS += $(XFLIB_DIR)/louvainmod/grappolo/src/coloringDistanceOne.cpp
HOST_SRCS += $(XFLIB_DIR)/louvainmod/grappolo/src/utilityClusteringFunctions.cpp
HOST_SRCS += $(XFLIB_DIR)/louvainmod/grappolo/src/parallelLouvainMethod.cpp
HOST_SRCS += $(XFLIB_DIR)/louvainmod/grappolo/src/parallelLouvainWithColoring.cpp
HOST_SRCS += $(XFLIB_DIR)/louvainmod/grappolo/src/louvainMultiPhaseRun.cpp
HOST_SRCS += $(XFLIB_DIR)/louvainmod/grappolo/src/parseInputParameters.cpp
HOST_SRCS += $(XFLIB_DIR)/louvainmod/grappolo/src/vertexFollowing.cpp
HOST_SRCS += $(XFLIB_DIR)/L2/louvainmod_2cu_ddr/partition/test.cpp
HOST_SRCS += $(XFLIB_DIR)/L2/louvainmod_2cu_ddr/partition/ParLV.cpp
HOST_SRCS += $(XFLIB_DIR)/L2/louvainmod_2cu_ddr/partition/ctrlLV.cpp
HOST_SRCS += $(XFLIB_DIR)/L2/louvainmod_2cu_ddr/partition/louvainPhase.cpp
HOST_SRCS += $(XFLIB_DIR)/L2/louvainmod_2cu_ddr/partition/partitionLouvain.cpp
HOST_SRCS += $(XFLIB_DIR)/ext/xcl2/xcl2.cpp
CXXFLAGS += -I$(XFLIB_DIR)/L2/include/hw
CXXFLAGS += -I$(XFLIB_DIR)/louvainmod/grappolo/include
#CXXFLAGS += -I$(XFLIB_DIR)/louvainmod/include
CXXFLAGS += -I$(XFLIB_DIR)/L2/louvainmod_2cu_ddr/partition
CXXFLAGS += -I$(XFLIB_DIR)/L2/louvainmod_2cu_ddr
#CXXFLAGS += -I$(XFLIB_DIR)/../utils/L1/include
CXXFLAGS += -I$(XFLIB_DIR)/ext/xcl2
CXXFLAGS += -I$(XFLIB_DIR)/L2/include
#CXXFLAGS += -I$(XFLIB_DIR)/../utils/L1/include
# please use "git submodule update --init --recursive"
CXXFLAGS += -I$(XFLIB_DIR)/ext/HLS_arbitrary_Precision_Types/include/

ifeq ($(TARGET),sw_emu)
CXXFLAGS += -D SW_EMU_TEST
endif

ifeq ($(TARGET),hw_emu)
CXXFLAGS += -D HW_EMU_TEST
endif

# ######################### Host compiler global settings ############################
CXXFLAGS += -I$(XILINX_XRT)/include -I$(XILINX_VITIS)/include -std=c++11 -O0 -Wall -Wno-unknown-pragmas -g
LDFLAGS += -L$(XILINX_XRT)/lib -lOpenCL -lpthread -lrt -Wno-unused-label -Wno-unused-variable -Wno-write-strings -Wno-narrowing -DVERBOSE -Wno-format
CXXFLAGS += -fmessage-length=0 -O0 -DPRAGMA -fopenmp -fPIC -DMULTITHREAD
CXXFLAGS += -I$(CUR_DIR)/src/ 

ifeq ($(HOST_ARCH), x86)
LDFLAGS += -L$(XILINX_VITIS)/lnx64/tools/fpo_v7_0 -Wl,--as-needed -lgmp
#-lmpfr -lIp_floating_point_v7_0_bitacc_cmodel
endif


# ################### Setting package and image directory #######################

EXE_NAME := host.exe
EXE_FILE := $(BUILD_DIR)/$(EXE_NAME)
HOST_ARGS :=  -x $(BUILD_DIR)/kernel_louvain.xclbin -f 3 -c -o -m 10 -prun $(XFLIB_DIR)/L2/louvainmod_2cu_ddr/data/example-wt.txt

# ##################### Kernel compiler global settings ##########################
VPP_FLAGS += -t $(TARGET) --platform $(XPLATFORM) --save-temps --optimize 2
VPP_FLAGS += --jobs 8

ifneq (,$(shell echo $(XPLATFORM) | awk '/u200/'))
VPP_FLAGS += --config $(CUR_DIR)/conn_u200.cfg
endif

ifneq (,$(shell echo $(XPLATFORM) | awk '/vu9p-f1/'))
VPP_FLAGS += --config $(CUR_DIR)/conn_aws_f1.cfg
endif

VPP_FLAGS += -I$(XFLIB_DIR)/L2/include
VPP_FLAGS += -I$(XFLIB_DIR)/../database/L1/include/hw
VPP_FLAGS += -I$(XFLIB_DIR)/../quantitative_finance/L1/include
VPP_FLAGS += -I$(XFLIB_DIR)/../utils/L1/include

VPP_FLAGS += -I$(XFLIB_DIR)/L2/include
VPP_FLAGS += -I$(XFLIB_DIR)/L2/include/hw
VPP_FLAGS += -I$(XFLIB_DIR)/../utils/L1/include
VPP_FLAGS += -I$(XFLIB_DIR)/L2/louvainmod_2cu_ddr
#VPP_FLAGS += --advanced.param compiler.userPostSysLinkOverlayTcl=$(CUR_DIR)/postSysLink.tcl

kernel_louvain_VPP_FLAGS +=  -D KERNEL_NAME=kernel_louvain
kernel_louvain_VPP_FLAGS += --hls.clock 250000000:kernel_louvain
VPP_LDFLAGS_kernel_louvain += --kernel_frequency 250


# ############################ Declaring Binary Containers ##########################

BINARY_CONTAINERS += $(BUILD_DIR)/kernel_louvain.xclbin
BINARY_CONTAINER_kernel_louvain_OBJS += $(TEMP_DIR)/kernel_louvain.xo

# ######################### Setting Targets of Makefile ################################
DATA_DIR += $(CUR_DIR)/data

.PHONY: all clean cleanall docs emconfig
all: check_vpp check_platform  $(EXE_FILE) $(BINARY_CONTAINERS) emconfig

.PHONY: host
host: check_xrt check_sysroot $(EXE_FILE)

.PHONY: xclbin
xclbin: check_vpp check_sysroot $(BINARY_CONTAINERS)

.PHONY: build
build: xclbin

# ################ Setting Rules for Binary Containers (Building Kernels) ################
$(TEMP_DIR)/kernel_louvain.xo: $(XFLIB_DIR)/L2/louvainmod_2cu_ddr/kernel_louvain.cpp
	$(ECHO) "Compiling Kernel: kernel_louvain"
	mkdir -p $(TEMP_DIR)
	$(VPP) -c $(kernel_louvain_VPP_FLAGS) $(VPP_FLAGS) -k kernel_louvain -I'$(<D)' --temp_dir $(TEMP_DIR) --report_dir $(TEMP_REPORT_DIR) -o'$@' '$<'


$(BUILD_DIR)/kernel_louvain.xclbin: $(BINARY_CONTAINER_kernel_louvain_OBJS)
	mkdir -p $(BUILD_DIR)
	$(VPP) -l $(VPP_FLAGS) --temp_dir $(BUILD_DIR) --report_dir $(BUILD_REPORT_DIR)/kernel_louvain $(VPP_LDFLAGS) $(VPP_LDFLAGS_kernel_louvain) -o '$@' $(+)

# Add kernel alias to xclbin post-build so that the same kernel targeting different devices can be differentiated.
	rm -f $(BUILD_DIR)/kernel_louvain_2cu_ddr_alias.xclbin
	$(XILINX_VITIS)/bin/xclbinutil -i $(BUILD_DIR)/kernel_louvain.xclbin \
								   --key-value=USER:kernel_louvain:'{"alias": "kernel_louvain_2cu_ddr"}' \
	                               -o $(BUILD_DIR)/kernel_louvain_2cu_ddr_alias.xclbin

# ################# Setting Rules for Host (Building Host Executable) ################
$(EXE_FILE): $(HOST_SRCS) | check_xrt
	mkdir -p $(BUILD_DIR)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS)

emconfig:$(EMCONFIG_DIR)/emconfig.json
$(EMCONFIG_DIR)/emconfig.json:
	emconfigutil --platform $(XPLATFORM) --od $(EMCONFIG_DIR)


# ###############Setting Essential Checks And Running Rules For Vitis Flow #############
run: all
ifeq ($(TARGET),$(filter $(TARGET),sw_emu hw_emu))
	$(CP) $(EMCONFIG_DIR)/emconfig.json .
	XCL_EMULATION_MODE=$(TARGET) $(EXE_FILE) $(HOST_ARGS)
else
	$(EXE_FILE) $(HOST_ARGS)
endif

# ################################# Cleaning Rules ##################################
cleanh:
	-$(RMDIR) $(EXE_FILE) vitis_* TempConfig system_estimate.xtxt *.rpt .run/
	-$(RMDIR) src/*.ll _xocc_* .Xil dltmp* xmltmp* *.log *.jou *.wcfg *.wdb sample_link.ini sample_compile.ini obj* bin* *.csv *.jpg *.jpeg *.png

cleank:
	-$(RMDIR) $(BUILD_DIR)/*.xclbin _vimage *xclbin.run_summary qemu-memory-_* emulation/ _vimage/ pl* start_simulation.sh *.xclbin
	-$(RMDIR) _x_temp.*/_x.* _x_temp.*/.Xil _x_temp.*/profile_summary.* xo_* _x*
	-$(RMDIR) _x_temp.*/dltmp* _x_temp.*/kernel_info.dat _x_temp.*/*.log 
	-$(RMDIR) _x_temp.* 

cleanall: cleanh cleank
	-$(RMDIR) $(BUILD_DIR)  build_dir.* emconfig.json *.html $(TEMP_DIR) $(CUR_DIR)/reports *.csv *.run_summary $(CUR_DIR)/*.raw package_* run_script.sh .ipcache *.str
	-$(RMDIR) $(XFLIB_DIR)/common/data/*.xe2xd* $(XFLIB_DIR)/common/data/*.orig*

	-$(RMDIR) $(AIE_CONTAINERS) $(CUR_DIR)/Work $(CUR_DIR)/*.xpe $(CUR_DIR)/hw.o $(CUR_DIR)/*.xsa $(CUR_DIR)/xnwOut aiesimulator_output .AIE_SIM_CMD_LINE_OPTIONS

clean: cleanh

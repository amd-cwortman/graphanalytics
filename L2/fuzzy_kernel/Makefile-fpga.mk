#
# Copyright 2019 Xilinx, Inc.
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
#

# -----------------------------------------------------------------------------
#                          project common settings

MK_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CUR_DIR := $(patsubst %/,%,$(dir $(MK_PATH)))

.SECONDEXPANSION:

# -----------------------------------------------------------------------------
#                            common setup

# MK_INC_BEGIN vitis_help.mk

.PHONY: help

help::
	@echo ""
	@echo "Makefile Usage:"
	@echo ""
	@echo "  make host xclbin TARGET=<sw_emu|hw_emu|hw> DEVICE=<FPGA platform>"
	@echo "      Command to generate the design for specified target and device."
	@echo ""
	@echo "      TARGET defaults to sw_emu."
	@echo ""
	@echo "      DEVICE is case-insensitive and support awk regex."
	@echo "      For example, \`make xclbin TARGET=hw DEVICE='u200.*qdma'\`"
	@echo "      It can also be an absolute path to platform file."
	@echo ""
	@echo "  make run TARGET=<sw_emu|hw_emu|hw> DEVICE=<FPGA platform>"
	@echo "      Command to run application in emulation."
	@echo ""
	@echo "  make clean "
	@echo "      Command to remove the generated non-hardware files."
	@echo ""
	@echo "  make cleanall"
	@echo "      Command to remove all the generated files."
	@echo ""

# MK_INC_END vitis_help.mk

# MK_INC_BEGIN vivado.mk

TOOL_VERSION ?= 2019.2

ifeq (,$(XILINX_VIVADO))
XILINX_VIVADO = /opt/xilinx/Vivado/$(TOOL_VERSION)
endif
export XILINX_VIVADO

.PHONY: check_vivado
check_vivado:
ifeq (,$(wildcard $(XILINX_VIVADO)/bin/vivado))
	@echo "Cannot locate Vivado installation. Please set XILINX_VIVADO variable." && false
endif

export PATH := $(XILINX_VIVADO)/bin:$(PATH)

# MK_INC_END vivado.mk

# MK_INC_BEGIN vitis.mk

TOOL_VERSION ?= 2021.2

ifeq (,$(XILINX_VITIS))
XILINX_VITIS = /opt/xilinx/Vitis/$(TOOL_VERSION)
endif
export XILINX_VITIS
.PHONY: check_vpp
check_vpp:
ifeq (,$(wildcard $(XILINX_VITIS)/bin/v++))
	@echo "Cannot locate Vitis installation. Please set XILINX_VITIS variable." && false
endif

ifeq (,$(XILINX_XRT))
XILINX_XRT = /opt/xilinx/xrt
endif
export XILINX_XRT
.PHONY: check_xrt
check_xrt:
ifeq (,$(wildcard $(XILINX_XRT)/lib/libxilinxopencl.so))
	@echo "Cannot locate XRT installation. Please set XILINX_XRT variable." && false
endif

export PATH := $(XILINX_VITIS)/bin:$(XILINX_XRT)/bin:$(PATH)

ifeq (,$(LD_LIBRARY_PATH))
LD_LIBRARY_PATH := $(XILINX_XRT)/lib
else
LD_LIBRARY_PATH := $(XILINX_XRT)/lib:$(LD_LIBRARY_PATH)
endif
ifneq (,$(wildcard $(XILINX_VITIS)/bin/ldlibpath.sh))
export LD_LIBRARY_PATH := $(shell $(XILINX_VITIS)/bin/ldlibpath.sh $(XILINX_VITIS)/lib/lnx64.o):$(LD_LIBRARY_PATH)
endif

# Target check
TARGET ?= sw_emu
ifeq ($(filter $(TARGET),sw_emu hw_emu hw),)
$(error TARGET is not sw_emu, hw_emu or hw)
endif

# MK_INC_END vitis.mk

# MK_INC_BEGIN vitis_set_platform.mk

ifneq (,$(wildcard $(DEVICE)))
# Use DEVICE as a file path
XPLATFORM := $(DEVICE)
else
# Use DEVICE as a file name pattern
DEVICE_L := $(shell echo $(DEVICE) | tr A-Z a-z)
# Match the name
ifneq (,$(PLATFORM_REPO_PATHS))
XPLATFORMS := $(foreach p, $(subst :, ,$(PLATFORM_REPO_PATHS)), $(wildcard $(p)/*/*.xpfm))
XPLATFORM := $(strip $(foreach p, $(XPLATFORMS), $(shell echo $(p) | awk '$$1 ~ /$(DEVICE_L)/')))
endif
ifeq (,$(XPLATFORM))
XPLATFORMS := $(wildcard $(XILINX_VITIS)/platforms/*/*.xpfm)
XPLATFORM := $(strip $(foreach p, $(XPLATFORMS), $(shell echo $(p) | awk '$$1 ~ /$(DEVICE_L)/')))
endif
ifeq (,$(XPLATFORM))
XPLATFORMS := $(wildcard /opt/xilinx/platforms/*/*.xpfm)
XPLATFORM := $(strip $(foreach p, $(XPLATFORMS), $(shell echo $(p) | awk '$$1 ~ /$(DEVICE_L)/')))
endif
endif

ifneq ($(findstring u50, $(DEVICE)), u50)
ifneq ($(findstring u200, $(DEVICE)), u200)
ifneq ($(findstring u250, $(DEVICE)), u250)
ifneq ($(findstring aws-vu9p-f1, $(DEVICE)), aws-vu9p-f1)
$(warning [WARNING]: This project has not been tested for $(DEVICE). It may or may not work.)
endif
endif
endif
endif

define MSG_PLATFORM
No platform matched pattern '$(DEVICE)'.
Avaialble platforms are: $(XPLATFORMS)
To add more platform directories, set the PLATFORM_REPO_PATHS variable.
endef
export MSG_PLATFORM

define MSG_DEVICE
More than one platform matched: $(XPLATFORM)
Please set DEVICE variable more accurately to select only one platform file. For example: DEVICE='u200.*xdma'
endef
export MSG_DEVICE

.PHONY: check_platform
check_platform:
ifeq (,$(XPLATFORM))
	@echo "$${MSG_PLATFORM}" && false
endif
ifneq (,$(word 2,$(XPLATFORM)))
	@echo "$${MSG_DEVICE}" && false
endif

XDEVICE := $(basename $(notdir $(firstword $(XPLATFORM))))

# MK_INC_END vitis_set_platform.mk

# -----------------------------------------------------------------------------
# BEGIN_XF_MK_USER_SECTION
# -----------------------------------------------------------------------------

XF_PROJ_ROOT ?= $(shell bash -c 'export MK_PATH=$(MK_PATH); echo $${MK_PATH%L2/*}')
XFLIB_DIR := $(abspath $(XF_PROJ_ROOT))

# -----------------------------------------------------------------------------

KSRC_DIR = $(XFLIB_DIR)/L2/fuzzy_kernel/kernel

XCLBIN_NAME := fuzzy_kernel

VPP_CFLAGS += -I$(XFLIB_DIR)/L2/fuzzy_kernel/kernel/include/

ifneq (,$(shell echo $(XPLATFORM) | awk '/u50/'))
KERNELS := fuzzy_kernel_1:fuzzy_kernel_1.cpp fuzzy_kernel_2:fuzzy_kernel_2.cpp
CXXFLAGS += -D USE_HBM
VPP_LFLAGS += --config $(CUR_DIR)/conn_u50.ini
else ifneq (,$(shell echo $(XPLATFORM) | awk '/u200/'))
KERNELS := fuzzy_kernel_1:fuzzy_kernel_1.cpp fuzzy_kernel_2:fuzzy_kernel_2.cpp
CXXFLAGS += -D USE_DDR
VPP_LFLAGS += --config $(CUR_DIR)/conn_u200.ini
else ifneq (,$(shell echo $(XPLATFORM) | awk '/u250_xdma/'))
CXXFLAGS += -D USE_DDR
KERNELS := fuzzy_kernel_1:fuzzy_kernel_1.cpp fuzzy_kernel_2:fuzzy_kernel_2.cpp
KERNELS += fuzzy_kernel_3:fuzzy_kernel_3.cpp fuzzy_kernel_4:fuzzy_kernel_4.cpp
VPP_LFLAGS += --config $(CUR_DIR)/conn_u250.ini
else ifneq (,$(shell echo $(XPLATFORM) | awk '/u250_gen3x16/'))
CXXFLAGS += -D USE_DDR
KERNELS := fuzzy_kernel_1:fuzzy_kernel_1.cpp fuzzy_kernel_2:fuzzy_kernel_2.cpp
KERNELS += fuzzy_kernel_3:fuzzy_kernel_3.cpp fuzzy_kernel_4:fuzzy_kernel_4.cpp
VPP_LFLAGS += --config $(CUR_DIR)/conn_azure_u250.ini
VPP_LFLAGS += --config /proj/xtools/dsv/projects/FaaS/security/xclbin_noencrypt/xclbin_noencrypt.ini
VPP_LFLAGS += --advanced.param compiler.acceleratorBinaryContent=dcp
else ifneq (,$(shell echo $(XPLATFORM) | awk '/aws-vu9p-f1/'))
KERNELS := fuzzy_kernel_1:fuzzy_kernel_1.cpp fuzzy_kernel_2:fuzzy_kernel_2.cpp
CXXFLAGS += -D USE_DDR
VPP_LFLAGS += --config $(CUR_DIR)/conn_aws_f1.ini
endif
# -----------------------------------------------------------------------------

EXE_NAME = host

HOST_ARGS = -xclbin $(XCLBIN_FILE) -l $(XFLIB_DIR)/data/ -i ./

SRC_DIR = $(CUR_DIR)/host

CXXFLAGS += -I $(XFLIB_DIR)/kernel/include -I$(XFLIB_DIR)/tests/fpga/host

SRCS = host.cpp

EXTRA_OBJS += xcl2

EXT_DIR = $(XFLIB_DIR)/tests/fpga
xcl2_SRCS = $(EXT_DIR)/host/xcl2.cpp
xcl2_HDRS = $(EXT_DIR)/host/xcl2.hpp
xcl2_CXXFLAGS = -I $(EXT_DIR)/tests/fpga/host

# -----------------------------------------------------------------------------
# END_XF_MK_USER_SECTION
# -----------------------------------------------------------------------------

.PHONY: all
all: xclbin

# MK_INC_BEGIN vitis_kernel_rules.mk

XCLBIN_DIR_BASE ?= xclbin$(tag)

XCLBIN_DIR_SUFFIX ?= _$(XDEVICE)_$(TARGET)

XO_DIR = _x_temp.$(TARGET).$(XDEVICE)
TEMP_DIR = $(XO_DIR)
BUILD_DIR := build_dir.$(TARGET).$(XDEVICE)

XFREQUENCY ?= 300

VPP = v++
VPP_CFLAGS += -I$(KSRC_DIR)
VPP_CFLAGS += --target $(TARGET) --platform $(XPLATFORM) --temp_dir $(TEMP_DIR) --save-temps
VPP_CFLAGS += --kernel_frequency $(XFREQUENCY) --report_level 2
VPP_LFLAGS += --optimize 2 --jobs 16 \
  --xp "vivado_param:project.writeIntermediateCheckpoints=1"

KERNEL_NAMES := $(foreach k,$(KERNELS),$(word 1, $(subst :, ,$(k))))
XO_FILES := $(foreach k,$(KERNEL_NAMES),$(XO_DIR)/$(k).xo)
XCLBIN_FILE ?= $(BUILD_DIR)/$(XCLBIN_NAME).xclbin

define kernel_src_dep
kernelname := $(word 1, $(subst :, ,$(1)))
kernelfile := $(if $(findstring :, $(1)),$(word 2, $(subst :, ,$(1))),$$(kernelname).cpp)
$$(kernelname)_SRCS := $(KSRC_DIR)/$$(kernelfile)
$$(kernelname)_SRCS += $$($$(kernelname)_EXTRA_SRCS)
endef

$(foreach k,$(KERNELS),$(eval $(call kernel_src_dep,$(k))))

define kernel_hdr_dep
kernelname := $(word 1, $(subst :, ,$(1)))
kernelfile := $(if $(findstring :, $(1)),$(basename $(word 2, $(subst :, ,$(1)))),$$(kernelname))
$$(kernelname)_HDRS := $$(wildcard $(KSRC_DIR)/$$(kernelfile).h $(KSRC_DIR)/$$(kernelfile).hpp)
$$(kernelname)_HDRS += $$($(1)_EXTRA_HDRS)
endef

$(foreach k,$(KERNELS),$(eval $(call kernel_hdr_dep,$(k))))

$(XO_DIR)/%.xo: VPP_CFLAGS += $($(*)_VPP_CFLAGS)
$(XO_DIR)/%.xo: $$($$(*)_SRCS) $$($$(*)_HDRS) | check_vpp
	@echo -e "----\nCompiling kernel $*..."
	mkdir -p $(XO_DIR)
	$(VPP) -o $@ --kernel $* --compile $(filter %.cpp,$^) \
		$(VPP_CFLAGS)

$(XCLBIN_FILE): $(XO_FILES) | check_vpp
	@echo -e "----\nCompiling xclbin..."
	mkdir -p $(BUILD_DIR)
	$(VPP) -o $@ --link $^ \
		$(VPP_CFLAGS) $(VPP_LFLAGS) \
		$(foreach k,$(KERNEL_NAMES),$($(k)_VPP_CFLAGS)) \
		$(foreach k,$(KERNEL_NAMES),$($(k)_VPP_LFLAGS))

.PHONY: xo xclbin

xo: check_vpp check_platform $(XO_FILES)

xclbin: check_vpp check_platform $(XCLBIN_FILE)

# MK_INC_END vitis_kernel_rules.mk

# MK_INC_BEGIN vitis_host_rules.mk

OBJ_DIR_BASE ?= obj
BIN_DIR_BASE ?= bin

BIN_DIR_SUFFIX ?= _$(XDEVICE)

OBJ_DIR = $(CUR_DIR)/$(OBJ_DIR_BASE)$(BIN_DIR_SUFFIX)
BIN_DIR = $(CUR_DIR)/$(BIN_DIR_BASE)$(BIN_DIR_SUFFIX)

CXX := xcpp
CC := gcc

CXXFLAGS += -std=c++14 -fPIC \
	-I$(SRC_DIR) -I$(XILINX_XRT)/include -I$(XILINX_VIVADO)/include \
	-Wall -Wno-unknown-pragmas -Wno-unused-label -pthread -g
CFLAGS +=
LDFLAGS += -pthread -L$(XILINX_XRT)/lib -lxilinxopencl
LDFLAGS += -L$(XILINX_VIVADO)/lnx64/tools/fpo_v7_0 -Wl,--as-needed -lgmp -lmpfr \
	   -lIp_floating_point_v7_0_bitacc_cmodel

OBJ_FILES = $(foreach s,$(SRCS),$(OBJ_DIR)/$(basename $(s)).o)

define host_hdr_dep
$(1)_HDRS := $$(wildcard $(SRC_DIR)/$(1).h $(SRC_DIR)/$(1).hpp)
$(1)_HDRS += $$($(1)_EXTRA_HDRS)
endef

$(foreach s,$(SRCS),$(eval $(call host_hdr_dep,$(basename $(s)))))

$(OBJ_DIR)/%.o: CXXFLAGS += $($(*)_CXXFLAGS)

$(OBJ_FILES): $(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $$($$(*)_HDRS) | check_vpp check_xrt check_platform
	@echo -e "----\nCompiling object $*..."
	mkdir -p $(@D)
	$(CXX) -o $@ -c $< $(CXXFLAGS)

EXTRA_OBJ_FILES = $(foreach f,$(EXTRA_OBJS),$(OBJ_DIR)/$(f).o)

$(EXTRA_OBJ_FILES): $(OBJ_DIR)/%.o: $$($$(*)_SRCS) $$($$(*)_HDRS) | check_vpp check_xrt check_platform
	@echo -e "----\nCompiling extra object $@..."
	mkdir -p $(@D)
	$(CXX) -o $@ -c $< $(CXXFLAGS)


# MK_INC_END vitis_host_rules.mk

# MK_INC_BEGIN vitis_test_rules.mk

# -----------------------------------------------------------------------------
#                                clean up

clean:
ifneq (,$(OBJ_DIR_BASE))
	rm -rf $(CUR_DIR)/$(OBJ_DIR_BASE)*
endif
ifneq (,$(BIN_DIR_BASE))
	rm -rf $(CUR_DIR)/$(BIN_DIR_BASE)*
endif

cleanx:
ifneq (,$(BIN_DIR_BASE))
	rm -rf $(CUR_DIR)/$(BIN_DIR_BASE)*/emconfig.json
endif

cleanall: clean cleanx
	rm -rf *.log plist 

# -----------------------------------------------------------------------------
#                                simulation run

$(BIN_DIR)/emconfig.json :
	emconfigutil --platform $(XPLATFORM) --od $(BIN_DIR)

ifeq ($(TARGET),sw_emu)
RUN_ENV += export XCL_EMULATION_MODE=sw_emu;
EMU_CONFIG = $(BIN_DIR)/emconfig.json
else ifeq ($(TARGET),hw_emu)
RUN_ENV += export XCL_EMULATION_MODE=hw_emu;
EMU_CONFIG = $(BIN_DIR)/emconfig.json
else ifeq ($(TARGET),hw)
RUN_ENV += echo "TARGET=hw";
EMU_CONFIG =
endif

.PHONY: run run_sw_emu run_hw_emu run_hw check

run_sw_emu:
	make TARGET=sw_emu run

run_hw_emu:
	make TARGET=hw_emu run

run_hw:
	make TARGET=hw run

run: host xclbin $(EMU_CONFIG)
	$(RUN_ENV) \
	$(EXE_FILE) $(HOST_ARGS)

check: run

# MK_INC_END vitis_test_rules.mk

.PHONY: build
build: xclbin host

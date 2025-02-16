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
#
#+-------------------------------------------------------------------------------
# The following parameters are assigned with default values. These parameters can
# be overridden through the make command line
#+-------------------------------------------------------------------------------

REPORT := no
PROFILE := no
DEBUG := no

#'estimate' for estimate report generation
#'system' for system report generation
ifneq ($(REPORT), no)
VPP_LDFLAGS += --report estimate
VPP_LDFLAGS += --report system
endif

#Generates profile summary report
ifeq ($(PROFILE), yes)
VPP_LDFLAGS += --profile_kernel data:all:all:all
endif

#Generates debug summary report
ifeq ($(DEBUG), yes)
#VPP_LDFLAGS += --dk protocol:all:all:all
endif

#Check environment setup
ifndef XILINX_VITIS
  XILINX_VITIS = /opt/xilinx/Vitis/$(TOOL_VERSION)
  export XILINX_VITIS
endif
ifndef XILINX_XRT
  XILINX_XRT = /opt/xilinx/xrt
  export XILINX_XRT
endif


B_NAME = $(shell dirname $(XPLATFORM))

#Setting VPP
VPP := v++

#Cheks for aiecompiler

.PHONY: check_vivado
check_vivado:
ifeq (,$(wildcard $(XILINX_VIVADO)/bin/vivado))
	@echo "Cannot locate Vivado installation. Please set XILINX_VIVADO variable." && false
endif

.PHONY: check_vpp
check_vpp:
ifeq (,$(wildcard $(XILINX_VITIS)/bin/v++))
	@echo "Cannot locate Vitis installation. Please set XILINX_VITIS variable." && false
endif

.PHONY: check_xrt
check_xrt:
ifeq (,$(wildcard $(XILINX_XRT)/lib/libxilinxopencl.so))
	@echo "Cannot locate XRT installation. Please set XILINX_XRT variable." && false
endif

# check target
ifeq ($(filter $(TARGET),sw_emu hw_emu hw),)
$(error TARGET is not sw_emu, hw_emu or hw)
endif

ifneq (,$(wildcard $(DEVICE)))
# Use DEVICE as a file path
XPLATFORM := $(DEVICE)
else
# Use DEVICE as a file name pattern
# 1. search paths specified by variable
ifneq (,$(PLATFORM_REPO_PATHS))
# 1.1 as exact name
XPLATFORM := $(strip $(foreach p, $(subst :, ,$(PLATFORM_REPO_PATHS)), $(wildcard $(p)/$(DEVICE)/$(DEVICE).xpfm)))
# 1.2 as a pattern
ifeq (,$(XPLATFORM))
XPLATFORMS := $(foreach p, $(subst :, ,$(PLATFORM_REPO_PATHS)), $(wildcard $(p)/*/*.xpfm))
XPLATFORM := $(strip $(foreach p, $(XPLATFORMS), $(shell echo $(p) | awk '$$1 ~ /$(DEVICE)/')))
endif # 1.2
endif # 1
# 2. search Vitis installation
ifeq (,$(XPLATFORM))
# 2.1 as exact name
XPLATFORM := $(strip $(wildcard $(XILINX_VITIS)/platforms/$(DEVICE)/$(DEVICE).xpfm))
# 2.2 as a pattern
ifeq (,$(XPLATFORM))
XPLATFORMS := $(wildcard $(XILINX_VITIS)/platforms/*/*.xpfm)
XPLATFORM := $(strip $(foreach p, $(XPLATFORMS), $(shell echo $(p) | awk '$$1 ~ /$(DEVICE)/')))
endif # 2.2
endif # 2
# 3. search default locations
ifeq (,$(XPLATFORM))
# 3.1 as exact name
XPLATFORM := $(strip $(wildcard /opt/xilinx/platforms/$(DEVICE)/$(DEVICE).xpfm))
# 3.2 as a pattern
ifeq (,$(XPLATFORM))
XPLATFORMS := $(wildcard /opt/xilinx/platforms/*/*.xpfm)
XPLATFORM := $(strip $(foreach p, $(XPLATFORMS), $(shell echo $(p) | awk '$$1 ~ /$(DEVICE)/')))
endif # 3.2
endif # 3
endif

define MSG_PLATFORM
No platform matched pattern '$(DEVICE)'.
Available platforms are: $(XPLATFORMS)
To add more platform directories, set the PLATFORM_REPO_PATHS variable or point DEVICE variable to the full path of platform .xpfm file.
endef
export MSG_PLATFORM

define MSG_DEVICE
More than one platform matched: $(XPLATFORM)
Please set DEVICE variable more accurately to select only one platform file, or set DEVICE variable to the full path of the platform .xpfm file.
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
#Check ends

#   device2xsa - create a filesystem friendly name from device name
#   $(1) - full name of device
device2xsa = $(strip $(patsubst %.xpfm, % , $(shell basename $(DEVICE))))

# Cleaning stuff
RM = rm -f
RMDIR = rm -rf

MV = mv -f
CP = cp -rf
ECHO:= @echo

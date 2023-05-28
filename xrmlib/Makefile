#
# Copyright 2020-2021 Xilinx, Inc.
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

SHELL := /bin/bash

#
# Define __PROFILING__ to turn on profiling message from compute units
#
ifndef XILINX_XRT
    XILINX_XRT = /opt/xilinx/xrt
    export XILINX_XRT
endif

.DELETE_ON_ERROR:
.PHONY: all

all: cppBuild

#
# Global Definitions
#

# Location of graphanalytics project
GRAPH_ANALYTICS_DIR = ..

#######################################################################################################################
#
# C++ API Library
#

# Define the target directories.

ifdef DEBUG
CPP_BUILD_DIR = Debug
CXXFLAGS += -O0 -g
else
CPP_BUILD_DIR = Release
CXXFLAGS += -O3 -DNDEBUG
endif


XRM_DIR = $(GRAPH_ANALYTICS_DIR)/ext/XRM
XRM_LIB_OBJ_DIR = $(XRM_DIR)/$(CPP_BUILD_DIR)/CMakeFiles/xrm.dir/src/lib

# File that triggers a rebuild of the .a
XRM_BUILD_DEP = $(realpath $(XRM_DIR)/$(CPP_BUILD_DIR)/libxrm.so)

LIB_SHORT_NAME = XilinxXrmLib
LIB_NAME = lib$(LIB_SHORT_NAME).a

OBJS_all = $(wildcard $(XRM_LIB_OBJ_DIR)/*.o)

#$(info $(XRM_LIB_OBJ_DIR))
#$(info $(OBJS_all))

.PHONY: cppBuild

cppBuild: $(CPP_BUILD_DIR) $(CPP_BUILD_DIR)/$(LIB_NAME)

# Create the target directory (if needed)
$(CPP_BUILD_DIR):
	mkdir -p $(CPP_BUILD_DIR)

$(CPP_BUILD_DIR)/$(LIB_NAME): $(XRM_BUILD_DEP) $(OBJS_all)
	ar ru $@ $(OBJS_all)
	ranlib $@

$(XRM_BUILD_DEP):
	@cd $(XRM_DIR); \
	set -e; \
	. $(XILINX_XRT)/setup.sh; \
	./build.sh

#######################################################################################################################
#
# Clean
#

.PHONY: clean

clean:
	rm -rf Debug Release

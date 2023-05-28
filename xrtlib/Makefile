#
# Copyright 2020-2022 Xilinx, Inc.
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

ifeq (, $(shell which sudo))
    $(error ERROR: No sudo in PATH)
endif

ifeq (, $(shell which lsb_release))
    $(error ERROR: No lsb_release in PATH)
endif

.DELETE_ON_ERROR:
.PHONY: all
all: cppBuild

#
# Global Definitions
#

# Version of XRT to use with graphanalytics products
XRT_GIT_VERSION = 202120.2.12.427

# Location of graphanalytics project
GRAPH_ANALYTICS_DIR = ..

OS_VERSION = $(shell lsb_release -is)-$(shell lsb_release -rs)

#######################################################################################################################
#
# XRT Library
#


XRT_PARENT_DIR = $(GRAPH_ANALYTICS_DIR)/ext
XRT_DIR_NAME = XRT-$(OS_VERSION)
XRT_DIR = $(XRT_PARENT_DIR)/$(XRT_DIR_NAME)
XRT_DEPS_INSTALLED_FILE = /opt/xilinx/apps/graphanalytics/.xrtdeps-installed
XRT_BOOST_DIR = $(XRT_DIR)/boost/xrt
XRT_TRIGGER_LIB = $(XRT_DIR)/build/Release/opt/xilinx/xrt/lib/libxrt_core.so

.PHONY: cppBuild
cppBuild: xrtdeps $(XRT_TRIGGER_LIB)

test:
	echo $(XRT_DIR)

# Test the rewriting of boost source
# test:
# 	@cd $(XRT_PARENT_DIR)/XRT.bak/boost/xrt/include/boost/process; \
# 	sed -n '47{s/std::move(\(.*\))/\1/;p}' < environment.hpp

# Dummy file installed per machine that indicates that XRT dependencies have been installed

.PHONY: xrtdeps
xrtdeps: $(XRT_DEPS_INSTALLED_FILE)

# $(info $(shell ls -l $(XRT_DEPS_INSTALLED_FILE)))
# $(info $(shell ls -ld $(XRT_DIR)))

$(XRT_DEPS_INSTALLED_FILE): $(XRT_DIR)
	@echo "INFO: Installing XRT dependencies.  Please enter the sudo password if requested."
	sudo -H ./xrtdeps.sh $(XRT_DIR) "$(shell cd $(XRT_DIR); git branch | grep '*')"

# Any of the many XRT libraries, used to check whether XRT needs to be built

$(XRT_TRIGGER_LIB): $(XRT_DIR) $(XRT_BOOST_DIR)
	cd $(XRT_DIR); \
	sed -i 's|/tools/batonroot/rodin/devkits|skip-cmc-bmc|' src/runtime_src/ert/CMakeLists.txt; \
	./build/build.sh -with-static-boost $(realpath $(XRT_BOOST_DIR))

# The build of static boost.  If not there, git clone boost and build it

$(XRT_BOOST_DIR): $(XRT_DIR)
	cd $(XRT_DIR); \
	yes Y | src/runtime_src/tools/scripts/boost.sh; \
	cd boost/xrt/include/boost/process; \
	sed -i '47s/std::move(\(.*\))/\1/' environment.hpp

# The XRT source dir.  If not there, git clone it and set to proper version for graphanalytics

$(XRT_DIR):
	cd $(XRT_PARENT_DIR); \
	git clone https://github.com/Xilinx/XRT $(XRT_DIR_NAME); \
	cd $(XRT_DIR_NAME); \
	git checkout $(XRT_GIT_VERSION)


#######################################################################################################################
#
# Clean
#

.PHONY: clean

clean:
	rm -rf $(XRT_DIR)

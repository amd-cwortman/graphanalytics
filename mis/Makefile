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

# Product name and version
STANDALONE_NAME=mis
PRODUCT_VER=$(strip $(shell cat VERSION))

.DELETE_ON_ERROR:
.PHONY: all	
all: stage
#all: stage cppBuild pythonApi  

# Common targets for all products (packaging)
include ../common/Makefile-common.mk

#######################################################################################################################
#
# C++ API Library
#

CCC = g++
CXX = g++
# -D PRINTINFO: print additional debug messages
CXXFLAGS = -std=c++14 -DPRAGMA -Ofast -fopenmp -fmessage-length=0  -fPIC -Wall -Wno-unknown-pragmas -Wno-unused-label \
    -Wno-unused-variable -Wno-sign-compare -Wno-unused-result -Wno-unused-but-set-variable \
    -Wno-narrowing -fmessage-length=0 

CXXFLAGS += -DMIS_numChannels=16
CXXFLAGS += -DMIS_vertexLimits=1024000

# Define the target directories.
#DEBUG = 1
ifdef DEBUG
CPP_BUILD_DIR = Debug
CXXFLAGS += -O0 -g
else
CPP_BUILD_DIR = Release
CXXFLAGS += -O3 -DNDEBUG
#CXXFLAGS += -O0 -g
endif

USE_F1 = 0
ifeq ($(USE_F1),1)
    CXXFLAGS += -D USE_F1
endif

GLIB_VERSION=1
CXXFLAGS += -D_GLIBCXX_USE_CXX11_ABI=$(GLIB_VERSION)

## Target: libMis.so
INCLUDES_libMis = \
	-Iinclude \
	-I$(GRAPH_ANALYTICS_DIR)/mis/include \
	-I$(XILINX_XRT)/include \
	-I$(GRAPH_ANALYTICS_DIR)/L2/include \
	-I$(GRAPH_ANALYTICS_DIR)/L3/include \
	-isystem $(GRAPH_ANALYTICS_DIR)/ext/HLS_arbitrary_Precision_Types/include \
	-I$(GRAPH_ANALYTICS_DIR)/ext/xcl2 \
	-I$(GRAPH_ANALYTICS_DIR)/ext \
	-I$(GRAPH_ANALYTICS_DIR)/common/include
#	-I$(GRAPH_ANALYTICS_DIR)/plugin/tigergraph/comdetect/udf/QueryUdf 
#	-I${JAVA_HOME}/include \
#	-I${JAVA_HOME}/include/linux

LDLIBS_libMis = \
	
LIB_SHORT_NAME = AMDMis
LIB_NAME = lib$(LIB_SHORT_NAME).so
LOADER_SHORT_NAME = AMDMis_loader
LOADER_NAME = lib$(LOADER_SHORT_NAME).a
LIB_STATIC_SHORT_NAME = AMDMisStatic
LIB_STATIC_NAME = lib$(LIB_STATIC_SHORT_NAME).so

#SRCS_L3 = $(GRAPH_ANALYTICS_DIR)/ext/xcl2/xcl2.cpp
#OBJS_L3 = $(addprefix $(CPP_BUILD_DIR)/,$(notdir $(SRCS_L3:.cpp=.o)))
# louvain dynamic load 

SRCS_loader = src/mis_loader.cpp
OBJS_loader = $(addprefix $(CPP_BUILD_DIR)/,$(notdir $(SRCS_loader:.cpp=.o)))
DEPS_loader = $(OBJS_loader:.o=.d)

SRCS_top = $(filter-out $(SRCS_loader),$(wildcard src/*.cpp))
#SRCS_top = $(wildcard src/*.cpp) 
OBJS_top = $(addprefix $(CPP_BUILD_DIR)/,$(notdir $(SRCS_top:.cpp=.o)))

SRCS_all = $(SRCS_top) $(SRCS_loader) #$(SRCS_L3) 
#SRCS_all = $(SRCS_L3) $(SRCS_top) 
OBJS_all = $(addprefix $(CPP_BUILD_DIR)/,$(notdir $(SRCS_all:.cpp=.o)))
DEPS_all = $(OBJS_all:.o=.d)

OBJS_libMis = $(OBJS_top) #$(OBJS_L3) 


.PHONY: cppBuild cppBuild2

# Make in a child process so that we can isolate the run that includes automatic header dependencies
cppBuild: $(CPP_BUILD_DIR)
	@make cppBuild2

#cppBuild2: $(CPP_BUILD_DIR)/$(LIB_NAME) 
cppBuild2: $(CPP_BUILD_DIR)/$(LIB_NAME) $(CPP_BUILD_DIR)/$(LIB_STATIC_NAME) $(CPP_BUILD_DIR)/$(LOADER_NAME)

# Create the target directory (if needed)
$(CPP_BUILD_DIR):
	mkdir -p $(CPP_BUILD_DIR)

#
# .so
#

# Library dependencies for building the .so's

LIB_BUILD_DEPS = -L$(XILINX_XRT)/lib -lpthread -lrt -Wno-unused-label -Wno-narrowing -DVERBOSE -fopenmp
# -lOpenCL 

ifeq ($(MEMALLOC),tcmalloc)
    LIB_DEPS += -ltcmalloc
endif

LIB_DYNAMIC_BUILD_DEPS = $(LIB_BUILD_DEPS) -lxrt_core -lxrt_coreutil
LIB_STATIC_BUILD_DEPS = $(LIB_BUILD_DEPS) -luuid

# Library dependencies for any executable linking with the .so

LIB_DYNAMIC_RUN_DEPS = $(LIB_DYNAMIC_BUILD_DEPS) -L $(CPP_BUILD_DIR) -l$(LIB_SHORT_NAME)
LIB_STATIC_RUN_DEPS = $(LIB_BUILD_DEPS) -L $(CPP_BUILD_DIR) -l$(LIB_STATIC_SHORT_NAME) -ldl
LIB_LOADER_RUN_DEPS = $(LIB_BUILD_DEPS) -ldl

# SHAREDLIB_FLAGS_libMis = -shared
SHAREDLIB_FLAGS_libMis = -fPIC -rdynamic -shared -w -Wl,--export-dynamic -Wunused-variable

# Dynamic .so

$(CPP_BUILD_DIR)/$(LIB_NAME): $(OBJS_libMis) $(DEPLIBS_libMis)
	$(LINK.cc) -o $@ $(OBJS_libMis) $(SHAREDLIB_FLAGS_libMis) $(LDLIBS_libMis) $(LIB_DYNAMIC_BUILD_DEPS)

# Static .so

LOCAL_XRT = $(GRAPH_ANALYTICS_DIR)/ext/XRT-$(OSDIST)-$(OSVER)
LOCAL_XRT_BOOST_DIR = $(LOCAL_XRT)/boost/xrt/lib
LOCAL_XRT_BOOST_LIB_NAMES = \
    libboost_filesystem-mt-x64.a \
    libboost_program_options-mt-x64.a \
    libboost_system-mt-x64.a
LOCAL_XRT_BOOST_LIBS = $(addprefix $(LOCAL_XRT_BOOST_DIR)/,$(LOCAL_XRT_BOOST_LIB_NAMES))

LOCAL_XRT_LIB_DIR = $(LOCAL_XRT)/build/$(CPP_BUILD_DIR)/opt/xilinx/xrt/lib
LOCAL_XRT_LIB_NAMES = libxrt++_static.a libxrt_core_static.a libxrt_coreutil_static.a
LOCAL_XRT_LIBS = $(addprefix $(LOCAL_XRT_LIB_DIR)/,$(LOCAL_XRT_LIB_NAMES))

$(LOCAL_XRT_LIBS) $(LOCAL_XRT_BOOST_LIBS) &:
	cd ../xrtlib; \
	make

$(CPP_BUILD_DIR)/$(LIB_STATIC_NAME): $(OBJS_libMis) $(DEPLIBS_libMis) $(LOCAL_XRT_LIBS) $(LOCAL_XRT_BOOST_LIBS)
	$(LINK.cc) -std=gnu++14 -o $@ $(OBJS_libMis) -static-libgcc -static-libstdc++ -Wl,-Bsymbolic \
	    $(SHAREDLIB_FLAGS_libMis) $(LDLIBS_libMis) $(LOCAL_XRT_LIBS) \
	    $(LOCAL_XRT_BOOST_LIBS) $(LIB_STATIC_BUILD_DEPS)

# loader .a

$(CPP_BUILD_DIR)/$(LOADER_NAME): $(OBJS_loader) $(DEPLIBS_loader)
	ar ru $@ $(OBJS_loader)
	ranlib $@

# Macro to create a .o rule and a .d rule for each .cpp

define BUILD_CPP_RULE

$(CPP_BUILD_DIR)/$(notdir $(basename $(1)).o): $(1)
	$(COMPILE.cc) $(INCLUDES_libMis) -o $$@ $$<

$(CPP_BUILD_DIR)/$(notdir $(basename $(1)).d): $(1)
	@set -e; \
	rm -f $$@; \
	$(COMPILE.cc) -MM -MT $(CPP_BUILD_DIR)/$(notdir $(basename $(1)).o) $(INCLUDES_libMis) -MF $$@.$$$$$$$$ $$<; \
        sed 's,\($(CPP_BUILD_DIR)/$(notdir $(basename $(1)).o)\)[ :]*,\1 $$@ : ,g' < $$@.$$$$$$$$ > $$@; \
	rm -f $$@.$$$$$$$$

endef

# Expand the macro for each source file

$(foreach src,$(SRCS_all),$(eval $(call BUILD_CPP_RULE,$(src))))

# Include generated header dependency rules, but only if building the JNI .so
# (This restriction prevents problems with the dependency on the Java API for the JNI header.)

ifeq ($(MAKECMDGOALS),cppBuild2)
-include $(DEPS_all)
endif

#######################################################################################################################
#
# XCLBIN
#
INC_ALL_XCLBINS=1

XCLBIN_LICENSE = $(GRAPH_ANALYTICS_DIR)/L2/LICENSE
# F1 XCLBIN 
L2_TOP_DIR_F1 = $(GRAPH_ANALYTICS_DIR)/L2/mis_kernel
L2_XDEVICE_F1 = xilinx_aws-vu9p-f1_shell-v04261818_201920_3
L2_BUILD_DIR_F1 = $(L2_TOP_DIR_F1)/build_dir.hw.$(L2_XDEVICE_F1)
XCLBIN_NAME_F1 = misKernel.awsxclbin
XCLBIN_FILE_F1 = $(L2_BUILD_DIR_F1)/$(XCLBIN_NAME_F1)
XCLBIN_INSTALLED_NAME_F1 = mis_xilinx_aws-vu9p-f1_shell-v04261818_201920_3.awsxclbin
XCLBIN_INSTALLED_F1 =/opt/amd/apps/agml/mis/$(PRODUCT_VER)/xclbin/$(XCLBIN_INSTALLED_NAME_F1)


# U200 XCLBIN 
L2_TOP_DIR_U200 = $(GRAPH_ANALYTICS_DIR)/L2/mis_kernel
L2_XDEVICE_U200 = xilinx_u200_gen3x16_xdma_1_202110_1
L2_BUILD_DIR_U200 = $(L2_TOP_DIR_U200)/build_dir.hw.$(L2_XDEVICE_U200)
XCLBIN_NAME_U200 = misKernel.xclbin
XCLBIN_FILE_U200 = $(L2_BUILD_DIR_U200)/$(XCLBIN_NAME_U200)
XCLBIN_INSTALLED_NAME_U200 = mis_xilinx_u200_gen3x16_xdma_1_202110_1.xclbin
XCLBIN_INSTALLED_U200 =/opt/amd/apps/agml/mis/$(PRODUCT_VER)/xclbin/$(XCLBIN_INSTALLED_NAME_U200)

# U50 XCLBIN 
L2_TOP_DIR_U50 = $(GRAPH_ANALYTICS_DIR)/L2/mis_kernel
L2_XDEVICE_U50 = xilinx_u50_gen3x16_xdma_201920_3
L2_BUILD_DIR_U50 = $(L2_TOP_DIR_U50)/build_dir.hw.$(L2_XDEVICE_U50)
XCLBIN_NAME_U50 = misKernel.xclbin
XCLBIN_FILE_U50 = $(L2_BUILD_DIR_U50)/$(XCLBIN_NAME_U50)
XCLBIN_INSTALLED_NAME_U50 = mis_xilinx_u50_gen3x16_xdma_201920_3.xclbin
XCLBIN_INSTALLED_U50 =/opt/amd/apps/agml/mis/$(PRODUCT_VER)/xclbin/$(XCLBIN_INSTALLED_NAME_U50)

# U55 XCLBIN 
L2_TOP_DIR_U55 = $(GRAPH_ANALYTICS_DIR)/L2/mis_kernel
L2_XDEVICE_U55 = xilinx_u55c_gen3x16_xdma_2_202110_1
L2_BUILD_DIR_U55 = $(L2_TOP_DIR_U55)/build_dir.hw.$(L2_XDEVICE_U55)
XCLBIN_NAME_U55 = misKernel.xclbin
XCLBIN_FILE_U55 = $(L2_BUILD_DIR_U55)/$(XCLBIN_NAME_U55)
XCLBIN_INSTALLED_NAME_U55 = mis_$(L2_XDEVICE_U55).xclbin
XCLBIN_INSTALLED_U55 =/opt/amd/apps/agml/mis/$(PRODUCT_VER)/xclbin/$(XCLBIN_INSTALLED_NAME_U55)

ifeq ($(USE_U55C),1)
	XCLBIN_BUILD_DIR=$(L2_TOP_DIR_U55)
else
	XCLBIN_BUILD_DIR=$(L2_TOP_DIR_U50)
endif

.PHONY: xclbin

xclbin:
	cd $(XCLBIN_BUILD_DIR); \
	source $(XILINX_XRT)/setup.sh; \
	export PLATFORM_REPO_PATHS=$(XILINX_PLATFORMS); \
	make build TARGET=hw
	
$(XCLBIN_FILE_U50):
	@if [ -r $(XCLBIN_INSTALLED_U50) ]; then \
	    echo "INFO: The XCLBIN file $(XCLBIN_FILE_U50) is missing.  Copying from installed area."; \
	    cp -r $(XCLBIN_INSTALLED_U50) $@; \
	else \
	    echo "INFO: The XCLBIN file $(XCLBIN_FILE_U50) is missing."; \
	    echo "Because building an XCLBIN file can be an all-day proposition, you have two choices for completing this build:"; \
	    echo "1. Copy a pre-built XCLBIN to the target location above.  (Recommended for developers)"; \
	    echo "2. Do 'make xclbin' and go for a very long coffee break.  (Recommended for build automation)"; \
    	false; \
	fi

$(XCLBIN_FILE_F1):
	@if [ -r $(XCLBIN_INSTALLED_F1) ]; then \
	    echo "INFO: The XCLBIN file $(XCLBIN_FILE_F1) is missing.  Copying from installed area."; \
	    cp -r $(XCLBIN_INSTALLED_F1) $@; \
	else \
	    echo "ERROR: The XCLBIN file $(XCLBIN_FILE_F1) is missing."; \
	    echo "Because building an XCLBIN file can be an all-day proposition, you have two choices for completing this build:"; \
	    echo "1. Copy a pre-built XCLBIN to the target location above.  (Recommended for developers)"; \
	    echo "2. Do 'make xclbin' and go for a very long coffee break.  (Recommended for build automation)"; \
	    false; \
	fi

$(XCLBIN_FILE_U55):
	@if [ -r $(XCLBIN_INSTALLED_U55) ]; then \
	    echo "INFO: The XCLBIN file $(XCLBIN_FILE_U55) is missing.  Copying from installed area."; \
	    cp -r $(XCLBIN_INSTALLED_U55) $@; \
	else \
	    echo "ERROR: The XCLBIN file $(XCLBIN_FILE_U55) is missing."; \
	    echo "Because building an XCLBIN file can be an all-day proposition, you have two choices for completing this build:"; \
	    echo "1. Copy a pre-built XCLBIN to the target location above.  (Recommended for developers)"; \
	    echo "2. Do 'make xclbin' and go for a very long coffee break.  (Recommended for build automation)"; \
	    false; \
	fi

#######################################################################################################################
#
# python API wrapper
#

PYTHON = python3
PYTHON_DIR = /usr/include/$(PYTHON)
PYTHONENV_NAME = pyvenv
PYTHONENV = ./$(PYTHONENV_NAME)/bin/$(PYTHON)
PYTHON_API_DIR = wrappers/python
PYTHON_LIB_NAME := AMDMisPython.so #$(shell python-config --extension-suffix)

SRC_FILE_NAMES_python = pythonapi.cpp
PYTHON_INC_FLAGS = $(shell python3-config --includes)
INCLUDES_python = \
    -I$(PYTHON_DIR) \
    -Iinclude \
	-I$(GRAPH_ANALYTICS_DIR)/ext \
	-I$(PYTHON_INC_FLAGS)

SRCS_python = $(addprefix $(PYTHON_API_DIR)/,$(SRC_FILE_NAMES_python))
OBJS_python = $(addprefix $(CPP_BUILD_DIR)/,$(notdir $(SRCS_python:.cpp=.o)))

LIB_PATH = $(CPP_BUILD_DIR)

LDFLAGS_python = -L$(LIB_PATH) -l$(LIB_SHORT_NAME)

# Add pybind11 includes and set library name after setting virtual env
ifeq ($(MAKECMDGOALS),pythonApi2)
INCLUDES_python += $(shell $(PYTHONENV) -m pybind11 --includes)
endif

SHAREDLIB_FLAGS_python = -shared

.PHONY: pythonApi pythonApi2

# Make in a child process so that we can isolate the run that includes automatic header dependencies
pythonApi: cppBuild $(PYTHONENV_NAME)
	@make pythonApi2

$(PYTHONENV_NAME):
	$(PYTHON) -m venv $(PYTHONENV_NAME);\
	$(PYTHONENV) -m pip install wheel; \
	$(PYTHONENV) -m pip install pybind11; \
	$(PYTHONENV) -m pip install python-config

pythonApi2: $(CPP_BUILD_DIR) $(CPP_BUILD_DIR)/$(PYTHON_LIB_NAME)

$(CPP_BUILD_DIR)/$(PYTHON_LIB_NAME): $(OBJS_python)
	$(LINK.cc) $^ -o $@ $(SHAREDLIB_FLAGS_python) $(LDFLAGS_python)

# Macro to create a .o rule and a .d rule for each .cpp

define BUILD_CPP_RULE

$(CPP_BUILD_DIR)/$(notdir $(basename $(1)).o): $(1)
	$(COMPILE.cc) $(INCLUDES_python) -o $$@ $$<

$(CPP_BUILD_DIR)/$(notdir $(basename $(1)).d): $(1)
	@set -e; \
	rm -f $$@; \
	$(COMPILE.cc) -MM -MT $(CPP_BUILD_DIR)/$(notdir $(basename $(1)).o) $(INCLUDES_python)   -MF $$@.$$$$$$$$ $$<; \
        sed 's,\($(CPP_BUILD_DIR)/$(notdir $(basename $(1)).o)\)[ :]*,\1 $$@ : ,g' < $$@.$$$$$$$$ > $$@; \
	rm -f $$@.$$$$$$$$

endef

# Expand the macro for each source file

$(foreach src,$(SRCS_python),$(eval $(call BUILD_CPP_RULE,$(src))))

#######################################################################################################################
#
# C++ Tests
#

TEST_DIR = tests

INCLUDES_test = \
    -Iinclude \
	-I$(XILINX_XRT)/include \
	-I$(GRAPH_ANALYTICS_DIR)/L2/include \
	-I$(GRAPH_ANALYTICS_DIR)/L3/include \
	-I$(GRAPH_ANALYTICS_DIR)/ext \
	-I$(GRAPH_ANALYTICS_DIR)/ext/xcl2 \
	-I$(GRAPH_ANALYTICS_DIR)/common/include \
	-isystem $(GRAPH_ANALYTICS_DIR)/ext/HLS_arbitrary_Precision_Types/include 


SRC_FILE_NAMES_test = \
    mis_test.cpp 

SRCS_test = $(addprefix $(TEST_DIR)/,$(SRC_FILE_NAMES_test))
OBJS_test = $(addprefix $(CPP_BUILD_DIR)/,$(notdir $(SRCS_test:.cpp=.o)))
DEPS_test = $(OBJS_test:.o=.d)

# List of all test executables to build
EXEC_FILE_NAMES_test = \
    mis_test \
    mis_test_static \
    mis_test_loader \


EXECS_test = $(addprefix $(CPP_BUILD_DIR)/,$(EXEC_FILE_NAMES_test))

.PHONY: cppTest cppTest2

# Make in a child process so that we can isolate the run that includes automatic header dependencies
cppTest: cppBuild
	@make cppTest2

cppTest2: $(EXECS_test)

# Test executables
LDFLAGS_test = -fPIC -w

$(CPP_BUILD_DIR)/mis_test: $(SRCS_test) $(CPP_BUILD_DIR)/$(LIB_NAME)
	$(LINK.cc) -o $@ $< $(LDFLAGS_test) $(LIB_DYNAMIC_RUN_DEPS) $(INCLUDES_test)

$(CPP_BUILD_DIR)/mis_test_static: $(SRCS_test) $(CPP_BUILD_DIR)/$(LIB_STATIC_NAME)
	$(LINK.cc) -o $@ $< $(LDFLAGS_test) $(LIB_STATIC_RUN_DEPS) $(INCLUDES_test)

$(CPP_BUILD_DIR)/mis_test_loader: $(SRCS_test) $(SRCS_loader) $(CPP_BUILD_DIR)/$(LIB_STATIC_NAME)
	$(LINK.cc) -o $@ $< $(LDFLAGS_test) -DXILINX_MIS_USE_STATIC_SO=1 $(SRCS_loader) \
	$(LIB_LOADER_RUN_DEPS) $(INCLUDES_test)


###############################################################################
# Test targets and parameters

deviceNames=u50
ifeq ($(deviceNames),u50)
    XCLBIN_RUN = $(CURDIR)/staging/xclbin/$(XCLBIN_INSTALLED_NAME_U50)
else ifeq ($(deviceNames),aws-f1)
    XCLBIN_RUN = $(CURDIR)/staging/xclbin/$(XCLBIN_INSTALLED_NAME_F1)
else ifeq ($(deviceNames),u200)
    XCLBIN_RUN = $(CURDIR)/staging/xclbin/$(XCLBIN_INSTALLED_NAME_U200)		
else ifeq ($(deviceNames),u55c)
    XCLBIN_RUN = $(CURDIR)/staging/xclbin/$(XCLBIN_INSTALLED_NAME_U55)		
endif

data_path=./examples/data

run: cppTest stage
	set -e; \
	. $(XILINX_XRT)/setup.sh && \
	LD_LIBRARY_PATH=$(PWD)/$(CPP_BUILD_DIR):$$LD_LIBRARY_PATH $(CPP_BUILD_DIR)/mis_test \
	    -xclbin $(XCLBIN_RUN) -d $(data_path)  --devices $(deviceNames) 

run-static: cppTest stage
	set -e; \
	. $(XILINX_XRT)/setup.sh && \
	LD_LIBRARY_PATH=$(PWD)/$(CPP_BUILD_DIR):$$LD_LIBRARY_PATH $(CPP_BUILD_DIR)/mis_test_static \
	    -xclbin $(XCLBIN_RUN) -d $(data_path)  --devices $(deviceNames) 

#	LD_LIBRARY_PATH=$(PWD)/$(CPP_BUILD_DIR):$$LD_LIBRARY_PATH totalview $(CPP_BUILD_DIR)/mis_test_static -a \
#	    -xclbin $(XCLBIN_RUN) -d $(data_path)  --devices $(deviceNames) 

run-loader: cppTest stage
	set -e; \
	. $(XILINX_XRT)/setup.sh && \
	LD_LIBRARY_PATH=$(PWD)/$(CPP_BUILD_DIR):$$LD_LIBRARY_PATH $(CPP_BUILD_DIR)/mis_test_loader \
	    -xclbin $(XCLBIN_RUN) -d $(data_path)  --devices $(deviceNames) 


run-staging-cpp: cppTest stage
	cd staging/examples/cpp && make run

# Macro to create a .o rule and a .d rule for each .cpp

define BUILD_CPP_RULE

$(CPP_BUILD_DIR)/$(notdir $(basename $(1)).o): $(1)
	$(COMPILE.cc) $(INCLUDES_test) -o $$@ $$<

$(CPP_BUILD_DIR)/$(notdir $(basename $(1)).d): $(1)
	@set -e; \
	rm -f $$@; \
	$(COMPILE.cc) -MM -MT $(CPP_BUILD_DIR)/$(notdir $(basename $(1)).o) $(INCLUDES_test) -MF $$@.$$$$$$$$ $$<; \
        sed 's,\($(CPP_BUILD_DIR)/$(notdir $(basename $(1)).o)\)[ :]*,\1 $$@ : ,g' < $$@.$$$$$$$$ > $$@; \
	rm -f $$@.$$$$$$$$

endef

# Expand the macro for each test source file

$(foreach src,$(SRCS_test),$(eval $(call BUILD_CPP_RULE,$(src))))

# Include generated header dependency rules, but only if building the JNI .so
# (This restriction prevents problems with the dependency on the Java API for the JNI header.)

ifeq ($(MAKECMDGOALS),cppTest2)
-include $(DEPS_test)
endif


#######################################################################################################################
#
# Staging
#

# Files to be direct-copied from source tree to staging area

STAGE_COPY_FILES = \
    VERSION \
    include/xilinxmis.hpp \
    src/mis_loader.cpp

define STAGE_COPY_RULE
$(STAGE_DIR)/$(1): $(1)
	cp -f $(1) $$(STAGE_DIR)/$(1)
endef
$(foreach f,$(STAGE_COPY_FILES),$(eval $(call STAGE_COPY_RULE,$(f))))

# Binary (C++) files

STAGE_LIB_FILES = \
    $(LIB_NAME) \
    $(LIB_STATIC_NAME) \
    $(LOADER_NAME)

ifneq ($(wildcard $(CPP_BUILD_DIR)/$(PYTHON_LIB_NAME)),)
    STAGE_LIB_FILES += $(PYTHON_LIB_NAME)
endif

define STAGE_LIB_RULE
$$(STAGE_DIR)/lib/$(1): $$(CPP_BUILD_DIR)/$(1)
	cp -f $$^ $$@
endef
$(foreach f,$(STAGE_LIB_FILES),$(eval $(call STAGE_LIB_RULE,$(f))))

# Example files - need to be zipped up

#STAGE_EXAMPLE_ZIP = examples.zip
STAGE_EXAMPLE_FILE_NAMES = \
	cpp/misdemo.cpp cpp/Makefile \
	python/misdemo.py python/run.sh \
	python/README.md python/jupyter/misdemo_notebook.ipynb \
	data/rowPtr.bin data/colIdx.bin data/infos.txt

STAGE_EXAMPLE_FILES = $(addprefix examples/,$(STAGE_EXAMPLE_FILE_NAMES))
STAGE_STAGED_EXAMPLE_FILES = $(addprefix $(STAGE_DIR)/,$(STAGE_EXAMPLE_FILES))

.PHONY: stage stage2 fill-example-stage stage-examples

#$(STAGE_DIR)/$(STAGE_EXAMPLE_ZIP): $(STAGE_EXAMPLE_FILES)
#	mkdir -p $(sort $(dir $(STAGE_STAGED_EXAMPLE_FILES)))
#	@make fill-example-stage
#	@cd $(STAGE_DIR) ; \
#	zip -r $(STAGE_EXAMPLE_ZIP) examples
#	rm -rf $(STAGE_DIR)/examples

stage-examples: $(STAGE_EXAMPLE_FILES)
	mkdir -p $(sort $(dir $(STAGE_STAGED_EXAMPLE_FILES)))
	@make fill-example-stage

fill-example-stage: $(STAGE_STAGED_EXAMPLE_FILES)
	cp VERSION $(STAGE_DIR)/examples/

$(STAGE_STAGED_EXAMPLE_FILES):
	cp -f $(patsubst $(STAGE_DIR)/%,%,$@) $@
	

# XCLBIN file
STAGE_XCLBIN_NAME_F1 = $(XCLBIN_INSTALLED_NAME_F1)
STAGE_XCLBIN_FILE_F1 = $(STAGE_DIR)/xclbin/$(STAGE_XCLBIN_NAME_F1)

STAGE_XCLBIN_NAME_U50 = $(XCLBIN_INSTALLED_NAME_U50)
STAGE_XCLBIN_FILE_U50 = $(STAGE_DIR)/xclbin/$(STAGE_XCLBIN_NAME_U50)

STAGE_XCLBIN_NAME_U200 = $(XCLBIN_INSTALLED_NAME_U200)
STAGE_XCLBIN_FILE_U200 = $(STAGE_DIR)/xclbin/$(STAGE_XCLBIN_NAME_U200)

STAGE_XCLBIN_NAME_U55 = $(XCLBIN_INSTALLED_NAME_U55)
STAGE_XCLBIN_FILE_U55 = $(STAGE_DIR)/xclbin/$(STAGE_XCLBIN_NAME_U55)

STAGE_XCLBIN_LICENSE = $(STAGE_DIR)/xclbin/LICENSE

$(STAGE_XCLBIN_LICENSE): $(XCLBIN_LICENSE)
	cp -f $^ $@

$(STAGE_XCLBIN_FILE_U200): $(XCLBIN_FILE_U200)
	cp -f $^ $@

$(STAGE_XCLBIN_FILE_F1): $(XCLBIN_FILE_F1)
	cp -f $^ $@

$(STAGE_XCLBIN_FILE_U50): $(XCLBIN_FILE_U50)
	cp -f $^ $@

$(STAGE_XCLBIN_FILE_U55): $(XCLBIN_FILE_U55)
	cp -f $^ $@


# Top-level packaging rule

STAGE_SUBDIR_NAMES = include lib src xclbin
STAGE_SUBDIRS = $(addprefix $(STAGE_DIR)/,$(STAGE_SUBDIR_NAMES))
STAGE_ALL_FILES = \
    $(STAGE_COPY_COMMON_FILES) \
    $(addprefix $(STAGE_DIR)/,$(STAGE_COPY_FILES)) \
    $(addprefix $(STAGE_DIR)/lib/,$(STAGE_LIB_FILES)) \
    $(STAGE_DIR)/$(STAGE_EXAMPLE_ZIP) \


# Conditionally add XCLBIN files
# add xclbin license
STAGE_ALL_FILES += $(STAGE_XCLBIN_LICENSE)

ifeq ($(INC_ALL_XCLBINS),1) 
#	STAGE_ALL_FILES += $(STAGE_XCLBIN_FILE_F1) $(STAGE_XCLBIN_FILE_U200) $(STAGE_XCLBIN_FILE_U50) $(STAGE_XCLBIN_FILE_U55)
	STAGE_ALL_FILES += $(STAGE_XCLBIN_FILE_F1) $(STAGE_XCLBIN_FILE_U50) $(STAGE_XCLBIN_FILE_U55)
else ifeq ($(deviceNames),u200)
    STAGE_ALL_FILES += $(STAGE_XCLBIN_FILE_U200)
else ifeq ($(deviceNames),u50)
    STAGE_ALL_FILES += $(STAGE_XCLBIN_FILE_U50)
else ifeq ($(deviceNames),u55c)
    STAGE_ALL_FILES += $(STAGE_XCLBIN_FILE_U55)
else ifeq ($(deviceNames),aws-f1)
    STAGE_ALL_FILES += $(STAGE_XCLBIN_FILE_F1)
endif



# make staging area in two steps so intermediate outputs can be included in the 
# staging output as well.
stage: pythonApi
	make stage2 INC_ALL_XCLBINS=$(INC_ALL_XCLBINS) deviceNames=$(deviceNames)

stage2: $(STAGE_SUBDIRS) stage-examples $(STAGE_ALL_FILES)

$(STAGE_SUBDIRS):
	mkdir -p $@

#######################################################################################################################
#
# Java Tests
#
.PHONY: javaTest

javaTest:
	cd test/java && ant


help: help-common
	@echo "  make run [deviceNames=u50]"
	@echo "    Run standalone test with statically linked executable"
	@echo ""
	@echo "  Run options:"
	@echo "  deviceNames  : Specify Alveo devices to use for computation. Supported devices:"
	@echo "                 u50 (default)"
	@echo "                 u55c"
	@echo "                 aws-f1"
	@echo "  totalEntities: total number of entities to match against(default=10000000)"
	@echo ""
	@echo "  make all"
	@echo "  Compile MIS lib and generate staging area"
	@echo "" 
	@echo "  make run-staging-cpp"
	@echo "  Run the CPP example from the staging area"
	@echo ""
	@echo "  Common options"
	@echo "  INC_ALL_XCLBINS: 1: Include all supported XCLBINs for staging (default). 0: Only incude target XCLBIN"
	@echo ""
	@echo "NOTES: To compile MIS on Redhat/CentOS 7.x you may need to enable a newer gcc by running the command below:"
	@echo "  scl enable devtoolset-9 bash"

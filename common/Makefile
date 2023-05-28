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

.DELETE_ON_ERROR:
.PHONY: all

all: cppTest

#
# Global Definitions
#

# Location of graphanalytics project
GRAPH_ANALYTICS_DIR = ..

# General output directory for intermediate build artifacts
BUILD_DIR = build

CCC = g++
CXX = g++
CXXFLAGS = -std=c++11 -fPIC -Wall -Wno-unknown-pragmas -Wno-unused-label \
           -Wno-narrowing -fmessage-length=0

# Define the target directories.

ifdef DEBUG
CPP_BUILD_DIR = Debug
CXXFLAGS += -O0 -g
else
CPP_BUILD_DIR = Release
CXXFLAGS += -O3 -DNDEBUG
endif


#######################################################################################################################
#
# C++ Tests
#

TEST_DIR = tests

INCLUDES_test = \
	-I$(GRAPH_ANALYTICS_DIR)/ext \


LDLIBS_test = \
	

SRC_FILE_NAMES_test = \
    XVectorTest.cpp \


SRCS_test = $(addprefix $(TEST_DIR)/,$(SRC_FILE_NAMES_test))
OBJS_test = $(addprefix $(CPP_BUILD_DIR)/,$(notdir $(SRCS_test:.cpp=.o)))
DEPS_test = $(OBJS_test:.o=.d)

# List of all test executables to build
EXEC_FILE_NAMES_test = \
    XVectorTest \

EXECS_test = $(addprefix $(CPP_BUILD_DIR)/,$(EXEC_FILE_NAMES_test))

.PHONY: cppTest cppTest2

# Make in a child process so that we can isolate the run that includes automatic header dependencies
cppTest: $(CPP_BUILD_DIR)
	@make cppTest2

cppTest2: $(EXECS_test)

# Create the target directory (if needed)
$(CPP_BUILD_DIR):
	mkdir -p $(CPP_BUILD_DIR)


# Test executables

LDFLAGS_test = -fPIC

$(CPP_BUILD_DIR)/XVectorTest: $(CPP_BUILD_DIR)/XVectorTest.o
	$(LINK.cc) -o $@ $< $(LDFLAGS_test)

.PHONY: run-tests
run-tests: cppTest $(EXECS_test)
	@echo "Running XVectorTest"; \
	$(CPP_BUILD_DIR)/XVectorTest

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

# Expand the macro for each source file

$(foreach src,$(SRCS_test),$(eval $(call BUILD_CPP_RULE,$(src))))

# Include generated header dependency rules, but only if building the JNI .so
# (This restriction prevents problems with the dependency on the Java API for the JNI header.)

ifeq ($(MAKECMDGOALS),cppTest2)
-include $(DEPS_test)
endif


#######################################################################################################################
#
# Clean
#

#### Clean target deletes all generated files ####
.PHONY: clean

clean:
	rm -rf Debug Release
	

#######################################################################################################################
#
# Help
#

.PHONY: help
help:
	@echo "Makefile usage:"
	@echo "  make"
	@echo "      Build all test programs"
	@echo ""
	@echo "  make clean"
	@echo "      Remove all build artifacts"
	@echo "" 	
	@echo "  make run-tests"
	@echo "      Run all tests"

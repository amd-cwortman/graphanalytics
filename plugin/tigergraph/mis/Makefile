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

# Product name and version: PRODUCT_NAME is the name for TigerGraph plugin
STANDALONE_NAME = mis
PRODUCT_NAME = mis

.DELETE_ON_ERROR:
.PHONY: all

all:
	@echo "Placeholder"

#all: stage


EXAMPLE_DIR = examples/travelplan

# Files to be direct-copied from plugin source tree to staging area

STAGE_COPY_FILES = \
    VERSION \
    \
    bin/set-plugin-vars.sh \
    \
    udf/xilinxMis.hpp \
    udf/xilinxMisImpl.hpp \
    \
    $(wildcard $(EXAMPLE_DIR)/data/travelplan2trucks100.csv) \
    $(wildcard $(EXAMPLE_DIR)/data/travelplan2workorders100.csv) \
    $(wildcard $(EXAMPLE_DIR)/data/travelplan2trucks2000.csv) \
    $(wildcard $(EXAMPLE_DIR)/data/travelplan2workorders2000.csv) \
	$(EXAMPLE_DIR)/README.md \
    $(EXAMPLE_DIR)/bin/common.sh \
    $(EXAMPLE_DIR)/bin/set-plugin-vars.sh \
    $(EXAMPLE_DIR)/udf/travelPlanDemo.hpp \
    $(EXAMPLE_DIR)/udf/travelPlanDemoImpl.hpp \
    $(EXAMPLE_DIR)/query/schema.gsql \
    $(EXAMPLE_DIR)/query/load.gsql \
    $(EXAMPLE_DIR)/query/build_edges.gsql \
	$(EXAMPLE_DIR)/query/tg_supply_chain_schedule.gsql \
	$(EXAMPLE_DIR)/query/xlnx_supply_chain_schedule.gsql \
    \
    $(EXAMPLE_DIR)/bin/run.sh \
    $(EXAMPLE_DIR)/jupyter/travelplan-jupyter-tg.ipynb \
    $(EXAMPLE_DIR)/jupyter/travelplan-python-tg.py \

# Example directories into which to copy example common files
STAGE_EXAMPLES_DIRS = $(EXAMPLE_DIR)

# Files that need the string ALVEO_TG_PLUGIN_PATH rewritten to the plug-in path
STAGE_PLUGIN_PATH_FILES = \
    $(EXAMPLE_DIR)/jupyter/travelplan-jupyter-tg.ipynb \
    $(EXAMPLE_DIR)/jupyter/travelplan-python-tg.py

include ../common/Makefile-common.mk

.PHONY: stage
stage: stage-common update-version

clean: clean-common

cleanall: cleanall-common

help: help-common



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
STANDALONE_NAME = agml
PRODUCT_NAME = agml
PRODUCT_VER=$(strip $(shell cat VERSION))
BUNDLE_DIR = $(CURDIR)
ABS_GRAPH_ANALYTICS_DIR = $(BUNDLE_DIR)/../../..

$(info "BUNDLE_DIR $(BUNDLE_DIR)")
$(info "ABS_GRAPH_ANALYTICS_DIR $(ABS_GRAPH_ANALYTICS_DIR)")

ALVEO_PRODUCT_LIST = \
    cosinesim \
    fuzzymatch \
    mis \
    louvainmod

PLUGIN_LIST= \
    comdetect \
    fuzzymatch \
    mis \
    recomengine


.DELETE_ON_ERROR:
.PHONY: all

all: stage

EXAMPLE_DIR = examples/comdetect


include ../common/Makefile-common.mk

.PHONY: stage
stage: stage-products
	cp VERSION staging
	cp bin/install.sh staging

.PHONY: stage-products
stage-products:
	@echo "INFO: Start stage-products..."
	rm -rf staging
	mkdir -p staging staging/alveo
	@alveoProductList=($(ALVEO_PRODUCT_LIST)); \
	echo "INFO: Staging Alveo products $${alveoProductList[@]}..."; \
	for i in "$${alveoProductList[@]}"; do \
	    echo "INFO: Staging $(ABS_GRAPH_ANALYTICS_DIR)/$$i..."; \
	    cd $(ABS_GRAPH_ANALYTICS_DIR)/$$i; make stage; \
	    mkdir $(BUNDLE_DIR)/staging/alveo/$$i; \
	    cp -rp $(ABS_GRAPH_ANALYTICS_DIR)/$$i/staging/* $(BUNDLE_DIR)/staging/alveo/$$i; \
	done; \
	\
	pluginList=($(PLUGIN_LIST)); \
	echo "INFO: Staging plug-ins $${pluginList[@]}..."; \
	for i in "$${pluginList[@]}"; do \
	    echo "INFO: Staging $(BUNDLE_DIR)/../$$i..."; \
	    cd $(BUNDLE_DIR)/../$$i; make stage BUNDLE_DIR=$(PRODUCT_NAME); \
	    mkdir $(BUNDLE_DIR)/staging/$$i; \
	    cp -rp $(BUNDLE_DIR)/../$$i/staging/* $(BUNDLE_DIR)/staging/$$i; \
	done

.PHONY: clean
clean: clean-common clean-products

.PHONY: clean-all
cleanall: cleanall-common clean-products

.PHONY: clean-products
clean-products:
	@echo "INFO: Start clean-products..."
	@pluginList=($(PLUGIN_LIST)); \
	echo "INFO: Cleaning plug-ins $${pluginList[@]}..."; \
	for i in "$${pluginList[@]}"; do \
	    echo "INFO: Cleaning $(BUNDLE_DIR)/../$$i..."; \
	    cd $(BUNDLE_DIR)/../$$i; make clean; \
	done; \
	\
	alveoProductList=($(ALVEO_PRODUCT_LIST)); \
	echo "INFO: Cleaning Alveo products $${alveoProductList[@]}..."; \
	for i in "$${alveoProductList[@]}"; do \
	    echo "INFO: Cleaning $(ABS_GRAPH_ANALYTICS_DIR)/$$i..."; \
	    cd $(ABS_GRAPH_ANALYTICS_DIR)/$$i; make clean cleanpy; \
	done


help-local:
	@echo ""
	@echo "  make clean"
	@echo "  Clean all proudcts"
	
help: help-common help-local help-common-options




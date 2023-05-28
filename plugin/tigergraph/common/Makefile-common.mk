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

SHELL=/bin/bash
#
# Global Definitions
#

## Location of graphanalytics project
GRAPH_ANALYTICS_DIR = ../../..

# Location of TigerGraph Plugin common files
TG_PLUGIN_COMMON_DIR = ../common

#######################################################################################################################
#
# Staging
#

STAGE_DIR = staging

define STAGE_COPY_RULE
$$(STAGE_DIR)/$(1): $(1)
	cp -f $(1) $$(STAGE_DIR)/$(1)
endef
$(foreach f,$(STAGE_COPY_FILES),$(eval $(call STAGE_COPY_RULE,$(f),)))

# Files to be direct-copied from TigerGraph plugin common source tree to staging area

STAGE_COPY_COMMON_FILES = \
    install.sh \
    \
    bin/common.sh \
	bin/gen-cluster-info.py \
    bin/install-plugin-cluster.sh \
	bin/update-tigergraph-config.sh \
    bin/install-plugin-node.sh\
    \
    udf/mergeHeaders.py \
    udf/prepExprFunctions.py

define STAGE_COPY_COMMON_RULE
$$(STAGE_DIR)/$(1): $(TG_PLUGIN_COMMON_DIR)/$(1)
	cp -f $(TG_PLUGIN_COMMON_DIR)/$(1) $$(STAGE_DIR)/$(1)
endef
$(foreach f,$(STAGE_COPY_COMMON_FILES),$(eval $(call STAGE_COPY_COMMON_RULE,$(f),)))

# Files to be direct-copied from TigerGraph plugin common EXAMPLES source tree to staging area

TG_PLUGIN_COMMON_EXAMPLES_DIR = $(TG_PLUGIN_COMMON_DIR)/examples

STAGE_COPY_COMMON_EXAMPLES_FILES = \
    bin/common-udf.sh \
    bin/install-udf.sh \
    bin/install-udf-cluster.sh \
    bin/install-udf-node.sh

STAGE_ALL_COMMON_EXAMPLES_FILES =

define STAGE_COPY_COMMON_EXAMPLES_FILE_RULE
$(STAGE_DIR)/$(2)/$(1): $(TG_PLUGIN_COMMON_EXAMPLES_DIR)/$(1)
	cp -f $(TG_PLUGIN_COMMON_EXAMPLES_DIR)/$(1) $(STAGE_DIR)/$(2)/$(1)
STAGE_ALL_COMMON_EXAMPLES_FILES += $(STAGE_DIR)/$(2)/$(1)
endef

define STAGE_COPY_COMMON_EXAMPLES_RULE
$(foreach f,$(STAGE_COPY_COMMON_EXAMPLES_FILES),$(eval $(call STAGE_COPY_COMMON_EXAMPLES_FILE_RULE,$(f),$(1))))
endef

$(foreach edir,$(STAGE_EXAMPLES_DIRS),$(eval $(call STAGE_COPY_COMMON_EXAMPLES_RULE,$(edir))))

# Top-level packaging rule

STAGE_ALL_FILES = \
    $(addprefix $(STAGE_DIR)/,$(STAGE_COPY_FILES)) \
    $(addprefix $(STAGE_DIR)/,$(STAGE_COPY_COMMON_FILES)) \
    $(STAGE_ALL_COMMON_EXAMPLES_FILES)

STAGE_SUBDIRS = $(sort $(dir $(STAGE_ALL_FILES)))

# Default target
all: stage

.PHONY: stage-common
stage-common: $(STAGE_DIR) $(STAGE_SUBDIRS) $(STAGE_ALL_FILES) update-version

# Target to rewrite staged files that contain the string ALVEO_TG_PLUGIN_PATH to a path relative to /opt/amd/apps
# In the plug-in makefile, set STAGE_PLUGIN_PATH_FILES to a list of files relative to $(STAGE_DIR)
#
# $(PRODUCT_NAME) should be set to the directory name of the installed plugin (for example, recomengine)

PRODUCT_VER=$(strip $(shell cat VERSION))
BASE_PLUGIN_PREFIX = agml/integration/Tigergraph-3.x
ifdef BUNDLE_DIR
    PLUGIN_PREFIX = $(BASE_PLUGIN_PREFIX)/$(BUNDLE_DIR)
else
    PLUGIN_PREFIX = $(BASE_PLUGIN_PREFIX)
endif

.PHONY:
update-version:
	@fileList=($(STAGE_PLUGIN_PATH_FILES)); \
	if [[ $${#fileList[@]} -gt 0 ]]; then \
	    echo "INFO: Rewriting ALVEO_TG_PLUGIN_PATH to $(PLUGIN_PREFIX)/$(PRODUCT_NAME)/$(PRODUCT_VER)"; \
	fi; \
	for i in "$${fileList[@]}"; do \
	    echo "   ...in $(STAGE_DIR)/$$i"; \
	    sed -i 's,ALVEO_TG_PLUGIN_PATH,$(PLUGIN_PREFIX)/$(PRODUCT_NAME)/$(PRODUCT_VER),g' $(STAGE_DIR)/$$i; \
	done

$(STAGE_DIR):
	mkdir -p $@

$(STAGE_SUBDIRS):
	mkdir -p $@

#######################################################################################################################
#
# Packaging
#

OSDIST = $(shell lsb_release -si)
OSVER = $(shell lsb_release -sr)
OSVER_MAJOR = $(shell lsb_release -sr | tr -dc '0-9.' | cut -d \. -f1)
OSVER_MINOR = $(shell lsb_release -sr | tr -dc '0-9.' | cut -d \. -f2)
OSVER_DIR=$(OSVER_MAJOR).$(OSVER_MINOR)
DIST_TARGET =
OSDISTLC =
verbose=0
ifeq ($(OSDIST),Ubuntu)
    DIST_TARGET = deb
	OSDISTLC = ubuntu
else ifeq ($(OSDIST),CentOS)
    DIST_TARGET = rpm
	OSDISTLC = centos
endif

ARCH = $(shell uname -p)
CPACK_PACKAGE_FILE_NAME= amd-$(PRODUCT_NAME)-tigergraph-$(PRODUCT_VER)-$(OSVER)-$(ARCH).$(DIST_TARGET)
DIST_INSTALL_DIR = $(GRAPH_ANALYTICS_DIR)/scripts/amd-agml-install/$(OSDISTLC)-$(OSVER_DIR)/$(STANDALONE_NAME)/
DIST_RELEASE = 0

.PHONY: dist

dist: stage
	@if [ $(DIST_RELEASE) == 1 ]; then \
		echo "INFO: Removing previous versions of the package and vclf"; \
		git rm -f $(DIST_INSTALL_DIR)/*-$(PRODUCT_NAME)-tigergraph-?.*.$(DIST_TARGET).vclf; \
        rm -f     $(DIST_INSTALL_DIR)/*-$(PRODUCT_NAME)-tigergraph-?.*.$(DIST_TARGET); \
	fi
	
	@if [ "$(DIST_TARGET)" == "" ]; then \
	    echo "Packaging is supported for only Ubuntu and CentOS."; \
	else \
	    echo "Packaging $(DIST_TARGET) for $(OSDIST)"; \
	    cd package; \
		make ; \
		cd - ; \
		mkdir -p $(DIST_INSTALL_DIR); \
		cp -f ./package/$(CPACK_PACKAGE_FILE_NAME) $(DIST_INSTALL_DIR); \
		echo "INFO: Package published to $(abspath $(DIST_INSTALL_DIR)/$(CPACK_PACKAGE_FILE_NAME))"; \
	fi

	@if [ $(DIST_RELEASE) == 1 ]; then \
		echo "INFO: Adding new package to vclf"; \
		vclf add $(DIST_INSTALL_DIR)/$(CPACK_PACKAGE_FILE_NAME); \
	fi
###############################################################################

ifdef sshKey
    SSH_KEY_OPT=-i $(sshKey)
endif
ifeq ($(verbose),1)
	VERBOSE_OPT=-v
endif

deviceNames=u50
.PHONY: install
install: stage
	./staging/install.sh $(SSH_KEY_OPT) $(VERBOSE_OPT) -d $(deviceNames)

#######################################################################################################################
#
# Clean
#

#### Clean target deletes all generated files ####
.PHONY: clean-common cleanall-common

clean-dist:
	@cd package; make clean

clean-common: clean-dist
	rm -rf $(STAGE_DIR)

cleanall: clean-common
	rm -f package/*.deb package/*.rpm

.PHONY: help-common
help-common:
	@echo "Makefile usage"
	@echo "  make"
	@echo "  Build staging files"
	@echo ""
	@echo "  make install"
	@echo "  Build staging files and install the plugin"
	@echo ""
	@echo "  make dist"
	@echo "  Build installation package (RPM or DEB) for the current OS and architecture"
	@echo ""
	@echo "  make clean-dist"
	@echo "  Clean distribution package files"
	@echo ""
	@echo "  make cleanall"
	@echo "  Clean distribution package files and all generated packages"

help-common-options:
	@echo ""
	@echo "Options"
	@echo "  sshKey=ssh-key-file: run make with an ssh key for the user tigergraph"
	@echo "  verbose=1|0: run install script in verbose mode"

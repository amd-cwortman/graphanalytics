# Copyright 2020 Xilinx, Inc.
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WANCUNCUANTIES ONCU CONDITIONS OF ANY KIND, either express or
# implied.
# See the License for the specific language governing permissions and
# limitations under the License.

deviceNames=u50
refNames=$(CURDIR)/data/ref-names.csv
refNamesCSV=$(abspath $(refNames))
newNames=$(CURDIR)/data/new-names.csv
newNamesCSV=$(abspath $(newNames))
graphName=entity_resolution_$(USER)

compileMode=1
runMode=2
printResult=0
INC_ALL_XCLBINS=1

ifdef sshKey
    SSH_KEY_OPT=-i $(sshKey)
endif	

ifeq ($(printResult),1) 
	RUN_OPTIONS += -l 
endif	


STANDALONE_NAME = fuzzymatch
EXAMPLE=entity-resolution
PLUGIN_DEPS = ../../../../../$(STANDALONE_NAME)/src/fuzzymatch.cpp \
              ../../../../../$(STANDALONE_NAME)/include/fuzzymatch.hpp \
              ../../udf/xilinxFuzzyMatch.hpp \
			  ../../udf/xilinxFuzzyMatchImpl.hpp

PLUGIN_TARGET = .install-plugin-done
TG_DATA_ROOT = $(shell cat ~tigergraph/.tg.cfg | jq .System.DataRoot | tr -d \")
EXAMPLE_UDF_DEPS = udf/entityResolution.hpp udf/entityResolutionImpl.hpp
EXAMPLE_UDF_TARGET = .install-udf-done

all: run

$(PLUGIN_TARGET): $(PLUGIN_DEPS)
	cd ../../../../../$(STANDALONE_NAME) && make DEBUG=$(DEBUG)
	@echo "-------------------------------------------------------"
	@echo "Installing plugin files into TigerGraph $(TG_DATA_ROOT)"
	@echo "-------------------------------------------------------"
	cd ../../ && \
	    make DEBUG=$(DEBUG) stage INC_ALL_XCLBINS=$(INC_ALL_XCLBINS) deviceNames=$(deviceNames) && \
		./staging/install.sh -v -f $(SSH_KEY_OPT) -d $(deviceNames) 
	touch $(PLUGIN_TARGET)

$(EXAMPLE_UDF_TARGET): $(PLUGIN_TARGET) $(EXAMPLE_UDF_DEPS)
	@echo "--------------------------------------------------------------"
	@echo "Installing application specific UDFs into TigerGraph software"
	@echo "--------------------------------------------------------------"
	cd ../../ && make DEBUG=$(DEBUG) stage && \
	./staging/examples/$(EXAMPLE)/bin/install-udf.sh -v -f $(SSH_KEY_OPT)
	touch $(EXAMPLE_UDF_TARGET)

test: $(EXAMPLE_UDF_TARGET)
	@echo "-------------------------------------------------------"
	@echo "Running quick gsql test"
	@echo "-------------------------------------------------------"
	gsql -g $(graphName) test.gsql
	time gsql -g $(graphName) "run query test()"

install-plugin: $(PLUGIN_TARGET)

.PHONY: run
run: $(EXAMPLE_UDF_TARGET)
	@echo "-------------------------------------------------------"
	@echo "Running run.sh from staging "
	@echo "-------------------------------------------------------"
	cd ../../ && make DEBUG=$(DEBUG) stage && \
	    ./staging/examples/$(EXAMPLE)/bin/run.sh -v \
	    -g $(graphName) -s $(refNamesCSV) -t $(newNamesCSV) \
		-c $(compileMode) -r $(runMode) $(RUN_OPTIONS)

help:
	@echo "Makefile usage:"
	@echo "make install-plugin [deviceNames=deviceNames]"
	@echo "    Install the plugin for the selected target device"
	@echo ""
	@echo "make run [deviceNames=deviceNames]"
	@echo "    Run the example"
	@echo ""
	@echo ""
	@echo "Parameter descriptions:"
	@echo "deviceNames : Target device name. Supported devices: "
	@echo "                u50 (default)"
	@echo "                u55c"
	@echo "                aws-f1"
	@echo "refNames    : A csv file with reference names. default=../data/ref-names.csv"
	@echo "newNames    : A csv file with new names. default=../data/new-names.csv"
	@echo "graphName   : graph name (default=swift_<username>"
	@echo "compileMode : 0: skip database creation and gsql compilation"
	@echo "              1: recreate database and compile all (default)"
	@echo "              2: only compile query gsql"
	@echo "runMode     : 0: Skip both CPU and Alveo run"
	@echo "              1: Run only on CPU"
	@echo "              2: Run only on Alveo (default)"
	@echo "              3: Run on both CPU and Alveo"
	@echo "printResult : 0: Do NOT print detailed results to JSON output"
	@echo "              1: Print detailed results to JSON output"
	@echo ""
	@echo "sshKey      : SSH key for user tigergraph"	
	@echo "INC_ALL_XCLBINS : 1: Include all supported XCLBINs for staging"
	@echo "                  0: Include target XCLBIN only"
	@echo ""

clean:
	cd ../../../../../$(STANDALONE_NAME) && make clean
	cd ../../ && make clean
	rm -f $(PLUGIN_TARGET)

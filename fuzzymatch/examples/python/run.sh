#!/bin/bash

# Copyright 2021 Xilinx, Inc.
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

#python3 -m venv venv
#. venv/bin/activate
#pip install pandas

SCRIPT=$(readlink -f $0)
SCRIPTPATH=`dirname $SCRIPT`

# Product version
export PRODUCT_VER=`cat $SCRIPTPATH/../VERSION`

# Location of cosine similarity Alveo product
export XF_PROJ_ROOT=$PWD/../../
test -d "$XILINX_FUZZYMATCH" || export XILINX_FUZZYMATCH=/opt/xilinx/apps/graphanalytics/fuzzymatch

# Location of XRT and XRM
test -d "$XILINX_XRT" || export XILINX_XRT=/opt/xilinx/xrt
#$test -d "$XILINX_XRM" || export XILINX_XRM=/opt/xilinx/xrm

# Location of the Python wrappers
export LIB_PATH=$XF_PROJ_ROOT/lib
test -d "$LIB_PATH" || export LIB_PATH=$XILINX_FUZZYMATCH/$PRODUCT_VER/lib
export PYTHONPATH=$LIB_PATH:$PYTHONPATH

# Location of xclbin path
export XCLBIN_PATH=$XF_PROJ_ROOT/xclbin
test -d "$XCLBIN_PATH" || export XCLBIN_PATH=$XILINX_FUZZYMATCH/$PRODUCT_VER/xclbin

# Setup Xilinx Tools
. $XILINX_XRT/setup.sh
#. $XILINX_XRM/setup.sh

# Location of the C++ library
export LD_LIBRARY_PATH=$LIB_PATH:$LD_LIBRARY_PATH
echo $LD_LIBRARY_PATH

if [[ "$1" == "jupyter"* ]]; then
    DEVICE="U50"
elif [ $# -eq 0 ]; then
    DEVICE="AWS"
else
    DEVICE=$1
fi

# Run the command
if [[ "${DEVICE}" == "U50" ]] ; then
    deviceNames="xilinx_u50_gen3x16_xdma_201920_3"
    xclbinFile="fuzzy_xilinx_u50_gen3x16_xdma_201920_3.xclbin"
elif [[ "${DEVICE}" == "U55C" ]] ; then
    deviceNames="xilinx_u55c_gen3x16_xdma_base_2"
    xclbinFile="fuzzy_xilinx_u55c_gen3x16_xdma_2_202110_1.xclbin"
elif [[ "${DEVICE}" == "AWS" ]]; then
    deviceNames="xilinx_aws-vu9p-f1_shell-v04261818_201920_2"
    xclbinFile="fuzzy_xilinx_aws-vu9p-f1_shell-v04261818_201920_2.awsxclbin"
fi

# export variables for Jupyter Notebook
export XCLBIN_FILE=$XCLBIN_PATH/$xclbinFile
export DEV_NAME=$deviceNames
export DATA_DIR=$SCRIPTPATH/../data
patternFile="../data/ref-names.csv"
inputFile="../data/new-names.csv"

#patternFile="/wrk/xsjhdnobkup4/vivianw/graphanalytics_wrk/dev/graphanalytics/fuzzymatch/5Party-ocp20/ncvr_numrec_1000000_modrec_2_ocp_20_myp_0_nump_5_7000.csv"
#inputFile="/wrk/xsjhdnobkup4/vivianw/graphanalytics_wrk/dev/graphanalytics/fuzzymatch/5Party-ocp20/ncvr_numrec_1000000_modrec_2_ocp_20_myp_1_nump_5_100.csv"

echo "patternFile is ${patternFile}" 
echo "inputFile is ${inputFile}"

# Run the command
if [[ "$1" == "jupyter"* ]]
then
  $*
else
  echo "INFO: python3 pythondemo.py  --deviceNames ${deviceNames}  --xclbin ${XCLBIN_PATH}/${xclbinFile} --pattern_file ${patternFile} --pattern_index 1 --inputFile ${inputFile} --inputIndex 1"
  #python3 pythondemo.py  --deviceNames $deviceNames  --xclbin $XCLBIN_FILE
  python3 pythondemo.py  --deviceNames $deviceNames  --xclbin $XCLBIN_FILE --pattern_file ${patternFile} --pattern_index 1 --inputFile ${inputFile} --inputIndex 1
  #$* --xclbin "$XCLBIN_FILE" --data_dir "$DATA_DIR" --deviceNames "$DEV_NAME"
fi



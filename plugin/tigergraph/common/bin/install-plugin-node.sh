#!/bin/bash
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

set -e

# Turn on tracing for debugging this script
#set -x

SCRIPT=$(readlink -f $0)
SCRIPTPATH=`dirname $SCRIPT`
PRODUCT_VER=`cat $SCRIPTPATH/../VERSION`
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

###############################################################################

# script options processing
device_name="u50"
use_tcmalloc=0
static_mode=0
uninstall=0
verbose=0
force=0

while getopts ":d:fhsuv" opt
do
case $opt in
    f) force=1;;
    d) device_name=$OPTARG;;
    s) static_mode=1;;
    u) uninstall=1;;
    v) verbose=1;;
    ?) echo "ERROR: Unknown option: -$OPTARG"; exit 1;;
esac
done

. $SCRIPTPATH/common.sh

# . $SCRIPTPATH/set-plugin-vars.sh
# already done in common.sh

if [ $verbose -eq 1 ]; then
    echo "INFO: Script is running with the settings below:"
    echo "      uninstall=$uninstall"
    echo "      device_name=$device_name"
fi

###############################################################################

if [ $verbose -eq 1 ]; then
    echo "INFO: Checking TigerGraph installation version and directory"
fi

# Temporary directory of include files to be included into main UDF header (ExprFunctions.hpp)
tg_version=$(basename $tg_root_dir)
if [ "$tg_version" == "3.1.0" ]; then
    tg_temp_include_dir=$tg_temp_root/gsql/codegen/udf
else
    tg_temp_include_dir=$tg_temp_root/gsql/codegen/QueryUdf
fi
echo "INFO: UDF codegen directory is $tg_temp_include_dir"

# Source directory for TigerGraph plugin
plugin_dir=$SCRIPTPATH/..
plugin_udf_dir=$plugin_dir/udf

# Files to install into TigerGraph application area
app_plugin_files="$pluginMainUdf $pluginHeaders $pluginExtraFiles"
app_alveo_product_files=$pluginAlveoProductHeaders

# Files to install into TigerGraph compilation area
compile_plugin_files="$pluginAlveoProductHeaders $pluginHeaders"

###############################################################################

#
# If uninstalling, remove UDFs from UDF file ExprFunctions.hpp and delete auxiliary files
#

if [[ $uninstall -eq 1 ]]; then
    # If there are product UDFs in the UDF file, uninstall them
    if [ -f $tg_udf_dir/ExprFunctions.hpp ] \
            && [ $(grep -c "mergeHeaders.*$pluginName" $tg_udf_dir/ExprFunctions.hpp) -gt 0 ]
    then
        if [ ! -f "$tg_udf_dir/mergeHeaders.py" ]; then
            cp $plugin_udf_dir/mergeHeaders.py $tg_udf_dir
        fi
        echo "INFO: Uninstalling $pluginAlveoProductName UDFs"
        mv $tg_udf_dir/ExprFunctions.hpp $tg_udf_dir/ExprFunctions.hpp.prev
        python3 $tg_udf_dir/mergeHeaders.py -u $tg_udf_dir/ExprFunctions.hpp.prev $pluginName \
             > $tg_udf_dir/ExprFunctions.hpp
    else
        if [ $verbose -eq 1 ]; then
            echo "INFO: $pluginAlveoProductName UDFs not found in UDF file ExprFunctions.hpp"
        fi
    fi

    echo "INFO: Uninstalling $pluginAlveoProductName auxiliary files"
    for i in $app_plugin_files $app_alveo_product_files; do
        rm -f $tg_udf_dir/${i##*/}
    done

    for i in $compile_plugin_files; do
        rm -f $tg_temp_include_dir/${i##*/}
    done

    rm -f $tg_udf_dir/$pluginLibName

    if [ ! -z "$pluginStaticLibName" ]; then
        rm -f $tg_udf_dir/$pluginStaticLibName
    fi

    rm -rf $tg_udf_xclbin_dir

    cp $tg_udf_dir/ExprFunctions.hpp $tg_temp_include_dir

    echo "INFO: $pluginAlveoProductName uninstalled."

    exit 0
fi

###############################################################################
#
# Install the plugin files to the current TigerGraph node
#


# If the XCLBIN is from sandbox, copy it to TigerGraph

if [[ $pluginAlveoProductNeedsInstall -eq 1 ]]; then
    if [ $verbose -eq 1 ]; then
        echo "INFO: Installing local Alveo Product from $pluginAlveoProductPath into TigerGraph"
    fi
    mkdir -p $tg_udf_xclbin_dir
    
    if [ ! -z "$pluginXclbinNameU50" ] && [ "$device_name" == "u50" ]; then
        cp -f $pluginAlveoProductXclbinPathU50 $tg_udf_xclbin_dir
    fi

    if [ ! -z "$pluginXclbinNameU55C" ] && [ "$device_name" == "u55c" ]; then
        cp -f $pluginAlveoProductXclbinPathU55C $tg_udf_xclbin_dir
    fi
    
    if [ ! -z "$pluginXclbinNameAwsF1" ] && [ "$device_name" == "aws-f1" ]; then
        cp -f $pluginAlveoProductXclbinPathAwsF1 $tg_udf_xclbin_dir
    fi

    cp -f $pluginAlveoProductLibDir/$pluginLibName $tg_udf_dir
    if [[ ! -z "$pluginStaticLibName" &&  -f $pluginAlveoProductLibDir/$pluginStaticLibName ]]; then
        cp -f $pluginAlveoProductLibDir/$pluginStaticLibName $tg_udf_dir
    fi

    if [ -f $pluginAlveoProductLibDir/icd/libxilinxopencl.so ]; then
        mkdir -p $tg_udf_dir/icd
        cp -f $pluginAlveoProductLibDir/icd/libxilinxopencl.so $tg_udf_dir/icd
    fi
fi

# save a copy of the original UDF Files

if [ ! -d "$tg_udf_dir.orig" ]; then
    cp -r $tg_udf_dir $tg_udf_dir.orig
    echo "INFO: Saved a copy of the original QueryUdf files in $tg_udf_dir.orig"
fi

# If the existing ExprFunctions.hpp has not been prepared for plugins, replace it
# with the base prepared version (containing just TG-supplied UDFs)

if [ $force -eq 1 ] || [ ! -f $tg_udf_dir/ExprFunctions.hpp ] || [ $(grep -c mergeHeaders $tg_udf_dir/ExprFunctions.hpp) -eq 0 ]; then
    echo "INFO: TigerGraph UDF file ExprFunctions.hpp has no plugin tags.  Installing base UDF file with tags"
    cp $plugin_udf_dir/prepExprFunctions.py $tg_udf_dir
    cp -f $tg_app_udf_dir/ExprFunctions.hpp $tg_udf_dir/ExprFunctions.hpp.orig
    python3 $tg_udf_dir/prepExprFunctions.py $tg_udf_dir/ExprFunctions.hpp.orig > $tg_udf_dir/ExprFunctions.hpp
fi

# copy stock ExprUtil.hpp to the new gsql UDF directory under TG data 
cp $tg_app_udf_dir/ExprUtil.hpp $tg_udf_dir

# Copy files to TigerGraph UDF area

echo "INFO: Installing $pluginAlveoProductName auxiliary files"
for i in $app_plugin_files; do
    cp -f $plugin_dir/$i $tg_udf_dir
done

for i in $app_alveo_product_files; do
    cp -f $pluginAlveoProductPath/$i $tg_udf_dir
done

# Create Xilinx graph store directory
mkdir -p $tg_data_root/xgstore

# Generate cluster configuration file
echo "INFO: Generate plugin configration file $tg_udf_dir/xilinx-plugin-config.json for $device_name"
echo "DEBUG: python3 $SCRIPTPATH/gen-cluster-info.py $tg_udf_dir/xilinx-plugin-config.json $tg_data_root $device_name"
python3 $SCRIPTPATH/gen-cluster-info.py $tg_udf_dir/xilinx-plugin-config.json $tg_data_root $device_name

# Substitute the XCLBIN path for PLUGIN_XCLBIN_PATH_[FPGA] in all files that need the substitution

for i in $pluginXclbinPathFiles; do
    # replace the longest string first
    sed -i "s|PLUGIN_XCLBIN_PATH_U55C|\"$runtimeXclbinPathU55C\"|" $tg_udf_dir/${i##*/}
    sed -i "s|PLUGIN_XCLBIN_PATH_AWSF1|\"$runtimeXclbinPathAwsF1\"|" $tg_udf_dir/${i##*/}
    sed -i "s|PLUGIN_XCLBIN_PATH_U50|\"$runtimeXclbinPathU50\"|" $tg_udf_dir/${i##*/}
done

# Substitute the config path for PLUGIN_CONFIG_PATH in all files that need the substituion
for i in $pluginConfigPathFiles; do
    sed -i "s|PLUGIN_CONFIG_PATH|\"$tg_udf_dir/xilinx-plugin-config.json\"|" $tg_udf_dir/${i##*/}
done

# If static .so is enabled, enable the macro PLUGIN_USE_STATIC_SO
if [ $static_mode -eq 1 ]; then
    for i in $pluginUseStaticSoFiles; do
        sed -i "s|//\\(#define PLUGIN_USE_STATIC_SO\\)|\\1|" $tg_udf_dir/${i##*/}
    done
fi

# Install plugin to ExprFunctions.hpp file

echo "INFO: Installing $pluginAlveoProductName UDFs"
cp $plugin_udf_dir/mergeHeaders.py $tg_udf_dir
mv $tg_udf_dir/ExprFunctions.hpp $tg_udf_dir/ExprFunctions.hpp.prev
python3 $tg_udf_dir/mergeHeaders.py $tg_udf_dir/ExprFunctions.hpp.prev $tg_udf_dir/${pluginMainUdf##*/} \
     > $tg_udf_dir/ExprFunctions.hpp

# Copy include files used by UDFs to TG build directory for TigerGraph to build the UDF library

mkdir -p $tg_temp_include_dir
for i in $compile_plugin_files; do
    cp -f $tg_udf_dir/${i##*/} $tg_temp_include_dir
done
cp $tg_udf_dir/ExprFunctions.hpp $tg_temp_include_dir
cp $tg_udf_dir/ExprUtil.hpp $tg_temp_include_dir

# Copy system libstdc++ for GPE to work around library compatibility issue
OSDIST=`lsb_release -i |awk -F: '{print tolower($2)}' | tr -d ' \t'`

if [[ $OSDIST == "ubuntu" ]]; then
    #cp /usr/lib/x86_64-linux-gnu/libstdc++.so.6* $HOME/libstd
    rm -rf $HOME/libstd
elif [[ $OSDIST == "centos" ]]; then
    mkdir -p $HOME/libstd
    cp /usr/lib64/libstdc++.so.6* $HOME/libstd
else 
    echo "ERROR: only Ubuntu and Centos are supported."
    exit 3
fi


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

#set -x 
#
# Alveo Product Variables
#
# The plugin depends on its corresponding Alveo Product.  The following variables
# determine the names and locations of the components of that Alveo Product.
#
# Alveo product names
pluginAlveoProductName="Xilinx Maximal Independent Set"
standaloneAlveoProductName="Xilinx Maximal Independent Set"

# Name of the directory containing the Alveo Product installation
pluginInstalledAlveoProductDirName=mis

# Where to find the git repo for the Alveo Product if it exists, relative to the repo dir
pluginLocalAlveoProductPath=mis/staging

# The name of supported XCLBIN files
pluginSupportedDevices="u50 u55c aws-f1"
pluginXclbinNameU50=mis_xilinx_u50_gen3x16_xdma_201920_3.xclbin
pluginXclbinNameAwsF1=mis_xilinx_aws-vu9p-f1_shell-v04261818_201920_3.awsxclbin
pluginXclbinNameU55C=mis_xilinx_u55c_gen3x16_xdma_2_202110_1.xclbin

# The name of the Alveo Product's .so file
pluginLibName=libAMDMis.so
pluginStaticLibName=libAMDMisStatic.so

# Set to 1 if the Alveo Product's .so needs to be added to LD_PRELOAD
pluginAlveoProductLibNeedsPreload=0

#
# Plugin Variables
#
# Variables for identifying the artifacts of the plug-in
#

# Name of the plugin, as found in mergeHeaders comments
pluginName=xilinxMis

# The main UDF file.  This file gets installed into ExprFunctions.hpp.
# It must contain mergeHeaders comments.  Also, any headers that this file
# depends on must be present in $pluginAlveoProductHeaders or $pluginHeaders.
# The path of this file is relative to the plugin top directory
pluginMainUdf=udf/xilinxMis.hpp

# List of header files to copy from the Alveo Product into the TigerGraph application area
# and UDF compilation area.  The paths are relative to $pluginAlveoProductPath.
pluginAlveoProductHeaders="include/xilinxmis.hpp include/xilinx_apps_common.hpp include/xilinx_apps_loader.hpp src/mis_loader.cpp"

# List of header files to copy from the plugin into the TigerGraph application area
# and UDF compilation area.  The paths are relative to the plugin top directory
# (the parent of this script's directory).
pluginHeaders=udf/xilinxMisImpl.hpp

# List of extra files to copy from the plugin into the TigerGraph application area.
# Unlike $pluginHeaders, these files are not copied to the compilation area.
# The paths are relative to the plugin top directory (parent of this script's directory).
pluginExtraFiles=

# List of files (name only, no path) that need to have the identifier PLUGIN_XCLBIN_PATH_[FPGA]
# replaced with the XCLBIN path.
pluginXclbinPathFiles="xilinxMis.hpp xilinxMisImpl.hpp"

# List of files (name only, no path) that need to have the identifier PLUGIN_CONFIG_PATH
# replaced with the config path.
pluginConfigPathFiles="xilinxMisImpl.hpp xilinxMis.hpp"

# List of files (name only, no path) that need to have the macro PLUGIN_USE_STATIC_SO
# replaced with "1" to turn on the use of the static .so
pluginUseStaticSoFiles="xilinxMisImpl.hpp"

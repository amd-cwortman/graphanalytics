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

SCRIPT=$(readlink -f $0)
SCRIPTPATH=`dirname $SCRIPT`

echo "--------------------------------------------------------------------------------"
echo "INFO: running $SCRIPT $@"
echo "--------------------------------------------------------------------------------"

RED='\033[0;31m'
NC='\033[0m' # No Color

if [ "$USER" != "tigergraph" ]; then
    echo "ERROR: This script must be run as user tigergraph."
    exit 1
fi

mem_alloc="tcmalloc"
debug_flag=0
device_name="xilinx_u50_gen3x16_xdma_201920_3"
xrt_profiling=0
static_mode=0
uninstall=0
verbose=0
node_flags=
reset_env=0
force=0
while getopts ":a:d:fgprsuv" opt
do
case $opt in
    a) mem_alloc=$OPTARG;;
    d) device_name=$OPTARG;;
    f) force=1; node_flags+=" -f";;
    g) debug_flag=1;;
    p) xrt_profiling=1;;
    r) reset_env=1;;
    s) static_mode=1; node_flags+=" -s";;
    u) uninstall=1; node_flags+=" -u";;
    v) verbose=1; node_flags+=" -v";;
    ?) echo "ERROR: Unknown option -$OPTARG"; exit 1;;  # pass through to sub-script
esac
done

node_flags+=" -d $device_name"
. $SCRIPTPATH/common.sh

if ! [ -x "$(command -v gadmin)" ]; then
    echo "ERROR: Cannot find TigerGraph installation. Please add the gadmin command to user tigergraph's path."
    exit 2
fi

if [ $verbose -eq 1 ]; then
    echo "INFO: Cluster script is running with the settings below:"
    echo "      mem_alloc=$mem_alloc"
    echo "      device_name=$device_name"
    echo "      force=$force"
    echo "      debug_flag=$debug_flag"
    echo "      xrt_profiling=$xrt_profiling"
    echo "      uninstall=$uninstall"
    echo "      reset_env=$reset_env"
fi

echo ""
echo "INFO: TigerGraph installation info:"
echo "    APP root is $tg_root_dir"
echo "    TEMP root is $tg_temp_root"
echo "    DATA root is $tg_data_root"
echo "    UDF source directory is $tg_udf_dir"
echo "INFO: Home is $HOME"

# Making sure TigerGraph services are running
echo ""
echo "--------------------------------------------------------------------------------"
echo "INFO: Starting all Tigergraph services"
echo "INFO: gadmin start all"
echo "--------------------------------------------------------------------------------"
gadmin start all

# run installation script on each node
echo ""
echo "--------------------------------------------------------------------------------"
echo "INFO: Running installation script on each node with option $node_flags"
echo "INFO: grun all ${SCRIPTPATH}/install-plugin-node.sh $node_flags"
echo "--------------------------------------------------------------------------------"

grun all "${SCRIPTPATH}/install-plugin-node.sh $node_flags"

# Uninstall


# build a dictionary of existing LD_PRELOAD and LD_LIBRARY_PATH entries
if [[ $reset_env == 1 ]]; then
    tgEnv=""
else
    tgEnv=$(gadmin config get GPE.BasicConfig.Env)
fi
preloadStr=$(sed "s/^LD_PRELOAD=\\([^;]*\\);.*/\\1/" <<< $tgEnv)
IFS=':' read -a preloadArray <<< $preloadStr
declare -A preloadDict
for i in "${preloadArray[@]}"; do 
    if [[ ! -z $i ]]; then 
        preloadDict[$i]=1
    fi
done

libPathStr=$(sed "s/.*LD_LIBRARY_PATH=\\([^;]*\\);.*/\\1/" <<< $tgEnv)
IFS=':' read -a libPathArray <<< $libPathStr
declare -A libPathDict
for i in "${libPathArray[@]}"; do
    if [[ ! -z $i ]]; then
        libPathDict[$i]=1
    fi
done

# remove items from the dictionaries that need special handling
tcmalloc_path=$tg_root_dir/dev/gdk/gsdk/include/thirdparty/prebuilt/dynamic_libs/gmalloc/tcmalloc/libtcmalloc.so
unset preloadDict['\$LD_PRELOAD']
unset preloadDict[$tcmalloc_path]
unset libPathDict['\$LD_LIBRARY_PATH']

if [[ $uninstall -eq 1 ]]; then
    # remove the product .so from the preload
    unset preloadDict[$runtimeLibDir/$pluginLibName]
    # for .deb/.rpm installed Alveo products, remove the lib path from LD_LIBRARY_PATH
    # for local (sandbox) Alveo products, leave the lib path, as it is common to all plugins
    if [ $pluginAlveoProductNeedsInstall -eq 1 ]; then
        unset libPathDict[$runtimeLibDir]
    fi
    echo "INFO: Restarting GPE service"
    gadmin restart gpe -y
    exit 1
fi

. $SCRIPTPATH/update-tigergraph-config.sh

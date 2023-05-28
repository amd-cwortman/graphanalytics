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
#set -x

SCRIPT=$(readlink -f $0)
SCRIPTPATH=`dirname $SCRIPT`

. $SCRIPTPATH/common.sh

static_mode=0
mem_alloc="tcmalloc"
xrt_profiling=0
reset_env=0
while getopts ":a:prs" opt
do
case $opt in
    a) mem_alloc=$OPTARG;;
    p) xrt_profiling=1;;
    r) reset_env=1;;
    s) static_mode=1; node_flags+=" -s";;
    ?) echo "ERROR: Unknown option -$OPTARG"; exit 1;;  # pass through to sub-script
esac
done

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

# copy gsql-client.jar to tmp directory
cp $tg_root_dir/dev/gdk/gsql/lib/gsql_client.jar /tmp/gsql_client.jar.tmp
echo "INFO: Apply environment changes to TigerGraph installation"
gadmin start all

# add the plugin's .so to the configuration
if [ $static_mode -ne 1 ]; then
    if [ $pluginAlveoProductLibNeedsPreload -eq 1 ]; then
        # add the product .so to the dictionary
        preloadDict[$runtimeLibDir/$pluginLibName]=0
    fi
fi
libPathDict[$runtimeLibDir]=1

libPathDict["$HOME/libstd"]=1
libPathDict["/opt/xilinx/xrt/lib"]=1
libPathDict["/opt/xilinx/xrm/lib"]=1
libPathDict["/usr/lib/x86_64-linux-gnu/"]=1

# rebuild the LD_PRELOAD string
#ld_preload="$HOME/libstd/libstdc++.so.6"
ld_preload=
for i in "${!preloadDict[@]}"; do
    ld_preload=$ld_preload:$(readlink -f $i);
done

# rebuild the LD_LIBRARY_PATH string
ld_lib_path=
for i in "${!libPathDict[@]}"; do
    ld_lib_path=$ld_lib_path:$(readlink -f $i);
done

if [ "$mem_alloc" == "tcmalloc" ]; then
    ld_preload="$ld_preload:$tcmalloc_path"
fi

gpe_config="LD_PRELOAD=$ld_preload;LD_LIBRARY_PATH=$ld_lib_path:\$LD_LIBRARY_PATH;CPUPROFILE=/tmp/tg_cpu_profiler;CPUPROFILESIGNAL=12;MALLOC_CONF=prof:true,prof_active:false;XILINX_XRT=/opt/xilinx/xrt;XILINX_XRM=/opt/xilinx/xrm"
if [ "$xrt_profiling" -eq 1 ]; then
    gpe_config="$gpe_config;XRT_INI_PATH=$PWD/../scripts/debug/xrt-profile.ini"
fi

gadmin config set GPE.BasicConfig.Env "$gpe_config"
# increase RESTPP timeout
gadmin config set RESTPP.Factory.DefaultQueryTimeoutSec 36000
gadmin config set GUI.HTTPRequest.TimeoutSec 36000


echo "INFO: Apply the new configurations to $gpe_config"
gadmin config apply -y
gadmin restart GPE RESTPP -y
gadmin config get GPE.BasicConfig.Env
gadmin config get RESTPP.Factory.DefaultQueryTimeoutSec






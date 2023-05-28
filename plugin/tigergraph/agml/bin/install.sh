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
PRODUCT_VER=`cat $SCRIPTPATH/VERSION`

verbose=0

function usage() {
    echo "Usage: $0 [options]"
    echo "Options:"
    echo "  -i sshKey        : SSH key for user tigergraph"    
    echo "  -d device-name   : Specify Alveo device name. Supported devices: "
    echo "     u50"
    echo "     u55c"
    echo "     aws-f1"        
#     for dev in $pluginSupportedDevices
#     do
#         echo "                       $dev"
#     done

    echo "  -u               : Uninstall all AGML plug-ins"
    echo "  -v               : Verbose output"
    echo "  -s               : Use static .so instead of the default dynamic .so (experimental)"
    echo "  -h               : Print this help message"

    if [[ $verbose -eq 1 ]]; then
        echo "  -a mem_alloc     : Change memory allocator default=tcmalloc. " 
        echo "  -f               : Force installation"
        echo "  -g               : Build plugin libraries with __DEBUG__"
        echo "  -p               : Turn on XRT profiling and timetrace"
    fi
}

hostname=$(hostname)
# set default ssh_key for tigergraph
if [ -f ~/.ssh/tigergraph_rsa ]; then
    ssh_key_flag="-i ~/.ssh/tigergraph_rsa"
fi

# Process command-line options

flags=
uninstall=0
device="notset"
while getopts ":a:d:fghi:psuv" opt
do
case $opt in
    i) ssh_key=$OPTARG; ssh_key_flag="-i $ssh_key";;
    a) flags="$flags -$opt $OPTARG";;
    d) device=$OPTARG; flags="$flags -$opt $device";;
    f|g|p|s) flags="$flags -$opt";;
    v) flags="$flags -$opt"; verbose=1;;    
    u) uninstall=1; flags="$flags -$opt";;
    h) usage; exit 1;;
    ?) echo "ERROR: Unknown option: -$OPTARG"; usage; exit 1;;
esac
done

# Check that a device has been set

if [[ "$device" == "notset" && $uninstall -ne 1 ]]; then
    echo "ERROR: Alveo device name must be set via -d option."
    usage
    exit 2
fi

if [[ "$USER" == "tigergraph" ]]; then
    # Announce whether we're installing or uninstalling

    if [[ $uninstall -eq 1 ]]; then
        echo "INFO: Uninstalling all graph analytics plug-ins."
    else
        echo "INFO: Installing all graph analytics plug-ins targetting $device"
    fi

    # Run through the plugins, installing or uninstalling each one in turn

    cd ${SCRIPTPATH}
    first_plugin=1
    for plugin in *; do
        if [[ -d $plugin && -x $plugin/install.sh ]]; then
            cd $plugin
            if [[ $first_plugin == 1 ]]; then
                # reset preload path for the first plugin
                ./install.sh $flags -r 
            else
                ./install.sh $flags
            fi
            first_plugin=0
            cd ..
        fi
    done
    echo "INFO: Completed processing all graph analytics plug-ins."

# The current user isn't "tigergraph": ssh to this same machine as tigergraph and re-run this script

else
    echo "--------------------------------------------------------------------------------"
    echo "INFO: Running installation as user \"tigergraph\" with $flags. "
    echo "Enter password for \"tigergraph\" if prompted."
    echo "INFO: command=ssh -tq $ssh_key_flag tigergraph@$hostname ${SCRIPT} $flags"    
    echo "--------------------------------------------------------------------------------"
    ssh -tq $ssh_key_flag tigergraph@$hostname ${SCRIPT} $flags
        # note: -t is so that read prompts appear, -q is to suppress "connection closed" message

    # Install the GSQL remote client, needed for users other than tigergraph

    if [[ $uninstall -ne 1 ]]; then
        # copy gsql_client.jar to current user's home
        mkdir -p $HOME/gsql_client
        cp /tmp/gsql_client.jar.tmp $HOME/gsql_client/gsql_client.jar
        echo "INFO: Remote GSQL client has been installed in $HOME/gsql_client"
    fi
fi


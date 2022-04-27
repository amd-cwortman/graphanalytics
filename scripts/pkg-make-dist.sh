#!/bin/bash

# Copyright 2022 Xilinx, Inc.
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


set -e
#set -x

OSDIST=`lsb_release -i |awk -F: '{print tolower($2)}' | tr -d ' \t'`
OSREL=`lsb_release -r |awk -F: '{print tolower($2)}' |tr -d ' \t' | awk -F. '{print $1*100+$2}'`

device="notset"
cosinesim=0
louvain=0
fuzzymatch=0
mis=0
while getopts ":d:aclfm" opt
do
case $opt in
    d) device=$OPTARG;;    
    a) cosinesim=1; louvain=1; fuzzymatch=1; mis=1;;
    c) cosinesim=1;;
    l) louvain=1;;
    f) fuzzymatch=1;;
    m) mis=1;;
    ?) echo "ERROR: Unknown option: -$OPTARG"; exit 1;;
esac
done

#if [[ "$device" == "notset" ]] ; then
#    echo "ERROR: Alveo device name must be set via -d option."
#    exit 1
#fi

if ! [[ -x "$(command -v vclf)" ]]; then
    # set up vclf
    echo "INFO: Setting up environment for vclf"
    if [[ $OSDIST == "ubuntu" ]]; then
        if (( $OSREL == 1804 )); then
            source /proj/gdba/tools/py3-venv-u18/bin/activate
        elif (( $OSREL == 2004 )); then
            source /proj/gdba/tools/py3-venv-u20/bin/activate
        else
            echo "ERROR: Ubuntu release version must be 18.04 or 20.04."
            exit 2
        fi
    elif [[ $OSDIST == "centos" ]]; then
        if (( $OSREL == 709 )); then
            echo "INFO: $OSREL"
        else
            echo "ERROR: CentOS release version must be 7.9."
            exit 2
        fi
    else 
        echo "ERROR: only Ubuntu and Centos are supported."
        exit 3
    fi
fi

echo "INFO: Using $(command -v vclf)"

if [[ $cosinesim -eq 1 ]] ; then
    cd ../cosinesim && make cleanall && make dist DIST_RELEASE=1 && cd -
    cd ../plugin/tigergraph/recomengine/ && make clean && make dist DIST_RELEASE=1 && cd -
fi

if [[ $louvain -eq 1 ]] ; then
    cd ../louvainmod && make cleanall && make dist DIST_RELEASE=1 && cd -
    cd ../plugin/tigergraph/comdetect/ && make clean && make dist DIST_RELEASE=1 && cd -
fi

if [[ $fuzzymatch -eq 1 ]] ; then
    cd ../fuzzymatch && make cleanall && make dist DIST_RELEASE=1 && cd -
    cd ../plugin/tigergraph/fuzzymatch/ && make clean && make dist DIST_RELEASE=1 && cd -
fi

if [[ $mis -eq 1 ]] ; then
    cd ../mis && make cleanall && make dist DIST_RELEASE=1 && cd -
    cd ../plugin/tigergraph/mis/ && make clean && make dist DIST_RELEASE=1 && cd -
fi

OSDIST=`lsb_release -i |awk -F: '{print tolower($2)}' | tr -d ' \t'`

if [[ $OSDIST == "ubuntu" ]]; then
    find . -name *.deb -exec stat -c "%y %n"  {}  \;;
elif [[ $OSDIST == "centos" ]]; then
    find . -name *.rpm -exec stat -c "%y %n"  {}  \;;
else 
    echo "ERROR: only Ubuntu and Centos are supported."
    exit 3
fi


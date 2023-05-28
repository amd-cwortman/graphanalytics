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

function usage() {
    echo "Usage: $0 [options]"
    echo "Options:"
    echo "  -a : Make distribution for all proudcts"
    echo "  -b : Make distribution for agml proudcts"
    echo "  -c : Make distribution for cosinesim proudcts"
    echo "  -l : Make distribution for louvainmod proudcts"
    echo "  -f : Make distribution for fuzzymatch proudcts"
    echo "  -m : Make distribution for mis proudcts"
    echo "  -h : Print this help message"
}

if [ "$#" -lt 1 ]; then
    usage
    exit 1
fi 


OSDIST=`lsb_release -i |awk -F: '{print tolower($2)}' | tr -d ' \t'`
OSREL=`lsb_release -r |awk -F: '{print tolower($2)}' |tr -d ' \t' | awk -F. '{print $1*100+$2}'`

echo "INFO: Making distribution package for $OSDIST $OSREL"

device="notset"
cosinesim=0
louvain=0
fuzzymatch=0
agml=0
mis=0
while getopts ":d:abclfmh" opt
do
case $opt in
    d) device=$OPTARG;;    
    a) cosinesim=1; louvain=1; fuzzymatch=1; mis=1; agml=1;;
    b) agml=1;;
    c) cosinesim=1; agml=1;;
    l) louvain=1; agml=1;;
    f) fuzzymatch=1; agml=1;;
    m) mis=1; agml=1;;
    h) usage; exit 0;;
    ?) echo "ERROR: Unknown option: -$OPTARG"; usage; exit 1;;
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
        if [[ $OSREL == 1804 ]]; then
            source /proj/gdba/tools/py3-venv-u18/bin/activate
        elif [[ $OSREL == 2004 ]]; then
            source /proj/gdba/tools/py3-venv-u20/bin/activate
        else
            echo "ERROR: Ubuntu release version must be 18.04 or 20.04."
            exit 2
        fi
    elif [[ $OSDIST == "centos" ]]; then
        if [[ $OSREL == 709 ]]; then
            source /proj/gdba/tools/py3-venv-c79/bin/activate
        elif [[ $OSREL == 802 ]]; then
            source /proj/gdba/tools/py3-venv-c82/bin/activate
        elif [[ $OSREL == 803 ]]; then
            source /proj/gdba/tools/py3-venv-c83/bin/activate
        else
            echo "ERROR: CentOS release version must be 7.9 or 8.2."
            exit 3
        fi
    else 
        echo "ERROR: only Ubuntu and Centos are supported."
        exit 4
    fi
fi

echo "INFO: Using $(command -v vclf)"

if [[ $cosinesim -eq 1 ]] ; then
    #cd ../cosinesim && make cleanall && make dist DIST_RELEASE=1 && cd -
    cd ../cosinesim && make dist DIST_RELEASE=1 && cd -
    #cd ../plugin/tigergraph/recomengine/ && make clean && make dist DIST_RELEASE=1 && cd -
    version=$(cat ../cosinesim/VERSION)
    python3 ./pkg-update-version.py cosinesim $version
fi

if [[ $louvain -eq 1 ]] ; then
    cd ../louvainmod && make cleanall && make dist DIST_RELEASE=1 && cd -
    #cd ../plugin/tigergraph/comdetect/ && make clean && make dist DIST_RELEASE=1 && cd -
    version=$(cat ../louvainmod/VERSION)
    python3 ./pkg-update-version.py louvainmod $version
fi

if [[ $fuzzymatch -eq 1 ]] ; then
    cd ../fuzzymatch && make cleanall && make dist DIST_RELEASE=1 && cd -
    #cd ../plugin/tigergraph/fuzzymatch/ && make clean && make dist DIST_RELEASE=1 && cd -
    version=$(cat ../fuzzymatch/VERSION)
    python3 ./pkg-update-version.py fuzzymatch $version
fi

if [[ $mis -eq 1 ]] ; then
    cd ../mis && make cleanall && make dist DIST_RELEASE=1 && cd -
    #cd ../plugin/tigergraph/mis/ && make clean && make dist DIST_RELEASE=1 && cd -
    version=$(cat ../mis/VERSION)
    python3 ./pkg-update-version.py mis $version
fi

if [[ $agml -eq 1 ]] ; then
    cd ../plugin/tigergraph/agml/ && make clean && make dist DIST_RELEASE=1 && cd -
    version=$(cat ../plugin/tigergraph/agml/VERSION)
    python3 ./pkg-update-version.py agml $version
fi

OSDIST=`lsb_release -i |awk -F: '{print tolower($2)}' | tr -d ' \t'`

if [[ $OSDIST == "ubuntu" ]]; then
    if [[ $cosinesim -eq 1 ]] ; then
        find . -name *cosinesim*.deb -exec stat -c "%y %n"  {}  \;;
        #find . -name *recomengine*.deb -exec stat -c "%y %n"  {}  \;;
    fi

    if [[ $louvain -eq 1 ]] ; then
        find . -name *louvainmod*.deb -exec stat -c "%y %n"  {}  \;;
        #find . -name *comdetect*.deb -exec stat -c "%y %n"  {}  \;;
    fi

    if [[ $fuzzymatch -eq 1 ]] ; then
        find . -name *fuzzymatch*.deb -exec stat -c "%y %n"  {}  \;;
    fi

    if [[ $mis -eq 1 ]] ; then
        find . -name *mis*.deb -exec stat -c "%y %n"  {}  \;;
    fi

    if [[ $agml -eq 1 ]] ; then
        find . -name *agml*.deb -exec stat -c "%y %n"  {}  \;;
    fi
elif [[ $OSDIST == "centos" ]]; then
    if [[ $cosinesim -eq 1 ]] ; then
        find . -name *cosinesim*.rpm -exec stat -c "%y %n"  {}  \;;
        #find . -name *recomengine*.rpm -exec stat -c "%y %n"  {}  \;;
    fi

    if [[ $louvain -eq 1 ]] ; then
        find . -name *louvainmod*.rpm -exec stat -c "%y %n"  {}  \;;
        #find . -name *comdetect*.rpm -exec stat -c "%y %n"  {}  \;;
    fi

    if [[ $fuzzymatch -eq 1 ]] ; then
        find . -name *fuzzymatch*.rpm -exec stat -c "%y %n"  {}  \;;
    fi

    if [[ $mis -eq 1 ]] ; then
        find . -name *mis*.rpm -exec stat -c "%y %n"  {}  \;;
    fi

    if [[ $agml -eq 1 ]] ; then
        find . -name *agml*.rpm -exec stat -c "%y %n"  {}  \;;
    fi        
else 
    echo "ERROR: only Ubuntu and Centos are supported."
    exit 3
fi


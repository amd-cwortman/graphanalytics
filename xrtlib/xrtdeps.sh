#!/bin/bash
#
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
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

set -e

# Turn on tracing for debugging this script
#set -x

SCRIPT=$(readlink -f $0)
SCRIPTPATH=`dirname $SCRIPT`

if [ $# -lt 2 ]; then
    echo "ERROR: Wrong number of arguments."
    echo "Usage: $0 xrt-dir xrt-version-string"
    exit 1
fi

depsInstalledFile=/opt/xilinx/apps/graphanalytics/.xrtdeps-installed
#depsInstalledFile=test.txt
xrtDir=$1
xrtVersionStr=$2

if [ ! -d $xrtDir ]; then
    echo "ERROR: XRT directory $xrtDir does not exist."
    exit 1
fi

echo "INFO: Installing XRT dependencies..."
$xrtDir/src/runtime_src/tools/scripts/xrtdeps.sh
mkdir -p ${depsInstalledFile%/*}
echo "XRT Version: $xrtVersionStr" > $depsInstalledFile

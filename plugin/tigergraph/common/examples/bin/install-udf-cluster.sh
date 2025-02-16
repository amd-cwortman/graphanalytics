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

if [ "$USER" != "tigergraph" ]; then
    echo "ERROR: This script must be run as user tigergraph."
    exit 1
fi

# Check command line for errors
. $SCRIPTPATH/common-udf.sh

grun all "${SCRIPTPATH}/install-udf-node.sh $*"

# Update TigerGraph configuration
echo "INFO: Update TigerGraph configuration for the selected example"
$SCRIPTPATH/../../../bin/update-tigergraph-config.sh -r


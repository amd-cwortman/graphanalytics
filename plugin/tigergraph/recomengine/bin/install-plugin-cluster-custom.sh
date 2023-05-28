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

#
# This script is always run as the user "tigergraph"
#
# Install or uninstall Recommendation Engine tuples
read -s -p "Enter user tigergraph's password for gsql client (default: tigergraph):" password
password=${password:-tigergraph}
echo $password

echo "" 
if [[ "$uninstall" -eq 1 ]]; then
    if [ $(gsql -u tigergraph -p $password "LS"   | grep -c XilCosinesimMatch) -gt 0 ]; then
        echo "INFO: Removing Recommendation Engine tuples"
        gsql "DROP TUPLE XilCosinesimMatch"
    fi
    echo ""
    echo "INFO: Xilinx FPGA acceleration plugin for Tigergraph has been uninstalled."
else
    if [ $(gsql -u tigergraph -p $password "LS"  | grep -c XilCosinesimMatch) -lt 1 ]; then
        echo "INFO: Adding Recommendation Engine tuples"
        gsql_status=$(gsql -u tigergraph -p $password "TYPEDEF TUPLE<Id VERTEX, score double> XilCosinesimMatch")
        echo "gsql_status=$gsql_status"
    fi
    echo ""
    echo "INFO: Xilinx FPGA acceleration plugin for Tigergraph has been installed."
fi


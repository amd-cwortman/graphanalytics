#!/usr/bin/env bash 

#
# Copyright 2021 Xilinx, Inc.
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
script_dir=`dirname $SCRIPT`

. $script_dir/common.sh

if [ "$compile_mode" -eq 1 ]; then
    echo "-------------------------------------------------------------------------"
    echo "Running schema.gsql"
    echo "gsql -u $username -p $password \"\$(cat $script_dir/../query/schema.gsql | sed \"s/@graph/$xgraph/\")\""
    echo "-------------------------------------------------------------------------"
    gsql -u $username -p $password "$(cat $script_dir/../query/schema.gsql | sed "s/@graph/$xgraph/")"

    echo "-------------------------------------------------------------------------"
    echo "Installing load.gsql"
    echo "gsql -u $username -p $password \"\$(cat $script_dir/../query/load.gsql | sed \"s/@graph/$xgraph/\")\""
    echo "-------------------------------------------------------------------------"
    gsql -u $username -p $password "$(cat $script_dir/../query/load.gsql | sed "s/@graph/$xgraph/")"

    echo "-------------------------------------------------------------------------"
    echo "Loading $files"
    echo "gsql -u $username -p $password -g $xgraph \"run loading job load_job USING blacklist_csv = \"$blacklist\", transaction_csv = \"$txdata\""
    echo "-------------------------------------------------------------------------"
    gsql -u $username -p $password -g $xgraph "run loading job load_job USING blacklist_csv = \"$blacklist\", transaction_csv = \"$txdata\""

    echo "-------------------------------------------------------------------------"
    echo "Install base queries"
    echo "gsql -u $username -p $password \"\$(cat $script_dir/../query/base.gsql | sed \"s/@graph/$xgraph/\")\""
    echo "-------------------------------------------------------------------------"
    gsql -u $username -p $password "$(cat $script_dir/../query/base.gsql | sed "s/@graph/$xgraph/")"

    echo "-------------------------------------------------------------------------"
    echo "Running insert dummy nodes for distributed alveo computing"
    gsql -u $username -p $password -g $xgraph "RUN QUERY insert_dummy_nodes($num_nodes)"
fi

if [ "$compile_mode" -eq 1 ] || [ "$compile_mode" -eq 2 ]; then
    echo "-------------------------------------------------------------------------"
    echo "Installing Fuzzy Match CPU and FPGA queries"
    echo "gsql -u $username -p $password -g $xgraph \"\$(cat $script_dir/../query/fuzzymatch.gsql | sed \"s/@graph/$xgraph/\")\""
    echo "-------------------------------------------------------------------------"
    gsql -u $username -p $password -g $xgraph "$(cat $script_dir/../query/fuzzymatch.gsql | sed "s/@graph/$xgraph/")"

    # IMPORTANT: DO NOT USE A NETWORK DRIVE FOR LOG FILES IN DISTRIBUTED QUERIES.
    # OTHERWISE EACH NODE WILL OVERWRITE IT
fi

echo "-------------------------------------------------------------------------"
echo "Run mode: $run_mode"

if [ "$run_mode" -eq 1 ] || [ "$run_mode" -eq 3 ]; then
    echo "Running fuzzymatch_cpu"
    echo gsql -u $username -p $password -g $xgraph \'run query fuzzymatch_cpu\(\)\'
    echo "-------------------------------------------------------------------------"
    START=$(date +%s%3N)
    time gsql -u $username -p $password -g $xgraph "run query fuzzymatch_cpu()"
    TOTAL_TIME=$(($(date +%s%3N) - START))
    echo "fuzzy_match_cpu runtime: " $tg_home
fi

# Run on FPGA
if [ "$run_mode" -eq 2 ] || [ "$run_mode" -eq 3 ]; then
    START=$(date +%s%3N)
    echo "Running fuzzymatch_alveo"
    echo gsql -u $username -p $password -g $xgraph \'run query fuzzymatch_alveo\(\)\'
    time gsql -u $username -p $password -g $xgraph "run query fuzzymatch_alveo()"
    TOTAL_TIME=$(($(date +%s%3N) - START))
    echo "fuzzy_match_alveo: " $TOTAL_TIME
fi

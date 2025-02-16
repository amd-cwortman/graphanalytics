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

# Uncomment below to echo all commands for debug
#set -x

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

    # set timeout of loading job to 1 hour
    time gsql -u $username -p $password -g $xgraph "SET QUERY_TIMEOUT=3600000 RUN LOADING JOB load_tp2tr USING tp2tr_infile=\"$tp2tr_data\""
    gsql -u $username -p $password -g $xgraph "DROP JOB load_tp2tr"
    time gsql -u $username -p $password -g $xgraph "SET QUERY_TIMEOUT=3600000 RUN LOADING JOB load_tp2wo USING tp2wo_infile=\"$tp2wo_data\""
    gsql -u $username -p $password -g $xgraph "DROP JOB load_tp2wo"
    echo "INFO: -------- $(date) load jobs completed. --------"

    #  build tp2tp edges
    echo " "
    echo "Install and Run query build_edges"
    gsql -u $username -p $password -g $xgraph "$(cat $script_dir/../query/build_edges.gsql | sed "s/@graph/$xgraph/")"
    echo "Running build_edges()"
    time gsql -u $username -p $password -g $xgraph "run query build_edges()"
fi

if [ "$compile_mode" -eq 1 ] || [ "$compile_mode" -eq 2 ]; then
    gsql -u $username -p $password -g $xgraph "$(cat $script_dir/../query/tg_supply_chain_schedule.gsql)"
    echo " "
    gsql -u $username -p $password -g $xgraph "$(cat $script_dir/../query/xlnx_supply_chain_schedule.gsql | sed "s/@graph/$xgraph/")"
fi

if [ "$run_mode" -eq 1 ] || [ "$run_mode" -eq 3 ]; then
    echo "Run query tg_supply_chain_schedule"
    tgrun=$(gsql -u $username -p $password -g $xgraph "SET QUERY_TIMEOUT=720000000 run query tg_supply_chain_schedule(\"travel_plan\", \"tp2tp\", 100, 0, False, \"/tmp/tg-mis_schedules.log\", \"/tmp/tg-mis_mdata.log\")")

    # get run time
    echo "$tgrun"
    tgtime=$(echo "$tgrun" | jq .results | jq .[1] | jq .ExecTimeInMs)
fi

# Run on FPGA
if [ "$run_mode" -eq 2 ] || [ "$run_mode" -eq 3 ]; then
    echo "Run query assign_ids"
    time gsql -u $username -p $password -g $xgraph "run query assign_ids(\"travel_plan\")"

    echo "Run query build_csr"
    time gsql -u $username -p $password -g $xgraph "run query build_csr(\"travel_plan\", \"tp2tp\")"

    echo "Run query supply_chain_schedule_alveo"
    xlnxrun=$(gsql -u $username -p $password -g $xgraph "run query supply_chain_schedule_alveo(\"travel_plan\", \"tp2tp\", 0, False, \"/tmp/xlnx-mis_schedules.log\", \"/tmp/xlnx-mis_mdata.log\")")

    # get run status and time
    echo "$xlnxrun"
    xlnxtime=$(echo "$xlnxrun" | jq .results | jq .[1] | jq .ExecTimeInMs)
    xlnxstatus=$(echo "$xlnxrun" | jq .results | jq .[4] | jq .StatusMessage)

    # Print speedup or results
    if [ "$xlnxstatus" == '"Success!"' ]; then
        if [ "$run_mode" -eq 3 ]; then
            python3 -c "print(f'\nAlveo Speedup over CPU = {($tgtime / $xlnxtime):.3f}X!\n')"
        fi
    else
        echo ""
        echo "Error: Xilinx query did not run correctly! Please run the CSR generation queries (assign_ids(), build_csr()) and re run supply_chain_schedule_alveo()."
    fi
fi

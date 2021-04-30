#/usr/bin/env bash

set -x
set -e

if [ ! -f "$HOME/.tg.cfg" ]; then
    echo "ERROR: This script only supports TigerGraph version 3.x"
    exit 1
fi
tg_app_root=$(cat $HOME/.tg.cfg | jq .System.AppRoot | tr -d \")
tg_temp_root=$(cat $HOME/.tg.cfg | jq .System.TempRoot | tr -d \")
tg_data_root=$(cat $HOME/.tg.cfg | jq .System.DataRoot | tr -d \")

# set up PATH for tigergraph commands
export PATH=$tg_root_dir/../cmd:$PATH

echo $tg_app_root
echo $tg_temp_root
echo $tg_data_root

Time=$(date +%s%3N)
echo "$tg_app_root/dev/gdk/MakeUdf.$Time"
# Create backups
grun all "cp $tg_app_root/dev/gdk/MakeUdf $tg_app_root/dev/gdk/MakeUdf.$Time"

# Override with new ones
grun all "cp $PWD/makefiles/MakeUdf $tg_app_root/dev/gdk"

grun all "cp $PWD/udf/QueryUdf/ExprFunctions.hpp $tg_data_root/gsql/udf"
grun all "cp $PWD/udf/QueryUdf/ExprUtil.hpp $tg_data_root/gsql/udf"
grun all "cp $PWD/udf/QueryUdf/xaidecl.cpp $tg_data_root/gsql/udf"
grun all "cp $PWD/udf/QueryUdf/xai.h $tg_data_root/gsql/udf"
grun all "cp $PWD/udf/QueryUdf/xailoader.hpp $tg_data_root/gsql/udf"

grun all "cp $PWD/udf/QueryUdf/ExprFunctions.hpp $tg_temp_root/gsql/codegen/udf"
grun all "cp $PWD/udf/QueryUdf/ExprUtil.hpp $tg_temp_root/gsql/codegen/udf"
grun all "cp $PWD/udf/QueryUdf/xaidecl.cpp $tg_temp_root/gsql/codegen/udf"
grun all "cp $PWD/udf/QueryUdf/xai.h $tg_temp_root/gsql/codegen/udf"
grun all "cp $PWD/udf/QueryUdf/xailoader.hpp $tg_temp_root/gsql/codegen/udf"
grun all "cp $PWD/../../../L3/lib/libgraphL3.so $tg_data_root/gsql/udf"

# TODO
#gpe_config="LD_PRELOAD=$LD_PRELOAD:/home2/tigergraph/tigergraph/app/3.1.0/dev/gdk/gsdk/include/thirdparty/prebuilt/dynamic_libs/gmalloc/tcmalloc/libtcmalloc.so;LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu:/opt/xilinx/xrt/lib:/opt/xilinx/xrm/lib:/home2/tigergraph/libstd:/data/dxgradb/tigergraph/tigergraph/data/gsql/udf:$LD_LIBRARY_PATH;CPUPROFILE=/tmp/tg_cpu_profiler;CPUPROFILESIGNAL=12;MALLOC_CONF=prof:true,prof_active:false;XILINX_XRT=/opt/xilinx/xrt;XILINX_XRM=/opt/xilinx/xrm"
#gadmin config set GPE.BasicConfig.Env "$gpe_config"
#echo "INFO: Apply the new configurations to $gpe_config"
#gadmin config apply -y
gadmin restart gpe -y
gadmin config get GPE.BasicConfig.Env
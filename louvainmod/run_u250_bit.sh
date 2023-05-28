#make run deviceNames=u250
set -e; \
. /opt/xilinx/xrt/setup.sh; \
. /opt/xilinx/xrm/setup.sh; \
LD_LIBRARY_PATH=/home/xilinxmarketing/workspace/exp/graphanalytics-dev/louvainmod/Release:$LD_LIBRARY_PATH Release/cppdemo -x /home/xilinxmarketing/workspace/exp/graphanalytics-dev/louvainmod/kernel_louvain_ddr_alias.bit.xclbin -kernel_mode 5 -num_devices 1 -devices u250 -num_level 100 -num_iter 100 -load_alveo_partitions Release/cppdemo-out/as-Skitter-wt-r100.par.proj -setwkr 0 -driverAlone

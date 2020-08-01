# To specify the platform, run:
#   xbutil validate

echo ---------------------------------------------------------------------------
echo 0 Preparing
tasks=$1
if [ "$tasks" == "" ]; then
    echo ERROR: task need to be specified like "build.sh <task_type>"
    echo task_type could be sw_emu/hw_emu/hw/all
    exit 1
fi
if [ "$tasks" != "sw_emu" ]; then
    if [ "$tasks" != "hw_emu" ]; then
        if [ "$tasks" != "hw" ]; then
            if [ "$tasks" != "all" ]; then
            echo ERROR: task_type should be sw_emu/hw_emu/hw/all
            exit 1
            fi
        fi
    fi
fi

if [ "$tasks" == "sw_emu" ]; then
    tasks=("sw_emu")
elif [ "$tasks" == "hw_emu" ]; then
    tasks=("hw_emu")
elif [ "$tasks" == "hw" ]; then
    tasks=("hw")
else
    tasks=("sw_emu" "hw_emu" "hw")
fi

workspace="<your directory>"
target_platform="xilinx_u280_xdma_201920_3"
src="../src"
host="host"
krnl="ImageStyleTransform"

cd $workspace
if [ $? -ne 0 ]; then
    echo ERROR when preparing
    exit 1
fi
echo pwd: `pwd`
echo workspace: "$workspace"
echo target_platform: "$target_platform"
echo src: "$src"
echo host: "$host"
echo krnl: "$krnl"

cat>design.cfg<<EOF
EOF
echo "platform=$target_platform">>design.cfg
echo "debug=1">>design.cfg
echo "profile_kernel=data:all:all:all">>design.cfg
echo "[connectivity]">>design.cfg
echo "nk=$krnl:1:$krnl_1">>design.cfg
cat>xrt.ini<<EOF
[Debug]
profile=true
timeline_trace=true
data_transfer_trace=fine
EOF

echo ---------------------------------------------------------------------------
echo 1 Building the Host Program
echo \$ g++ -I$XILINX_XRT/include/ -I$XILINX_VIVADO/include/ -Wall -O0 -g -std=c++11 ./src/host.cpp  -o "$host"  -L$XILINX_XRT/lib/ -lOpenCL -lpthread -lrt -lstdc++
g++ -I$XILINX_XRT/include/ -I$XILINX_VIVADO/include/ -Wall -O0 -g -std=c++11 $src/$host.cpp  -o "$host"  -L$XILINX_XRT/lib/ -lOpenCL -lpthread -lrt -lstdc++

if [ $? -ne 0 ]; then
    echo ERROR when tried to build the host program
    exit 1
fi

for task in ${tasks[@]};do
    echo ---------------------------------------------------------------------------
    echo 2 Building the FPGA Binary
    echo 2.1 Compiling the hardware kernel
    echo \$ v++ -t $task --config design.cfg -c -k $krnl -I"$src" -o"$krnl.${target_platform}.xo" "$src/$krnl.cpp"
    v++ -t $task --config design.cfg -c -k $krnl -I"$src" -o"$krnl.${target_platform}.xo" "$src/$krnl.cpp"

    if [ $? -ne 0 ]; then
        echo ERROR when tried to compile the hardware kernel
        exit 1
    fi

    echo ---------------------------------------------------------------------------
    echo 2.2 linking the hardware kernel
    echo v++ -t $task --config design.cfg -l -o"$krnl.$task.${target_platform}.xclbin" $krnl.${target_platform}.xo
    v++ -t $task --config design.cfg -l -o"$krnl.$task.${target_platform}.xclbin" $krnl.${target_platform}.xo
    if [ $? -ne 0 ]; then
        echo ERROR when tried to link the hardware kernel
        exit 1
    fi

    echo ---------------------------------------------------------------------------
    emconfigutil --platform $target_platform
    if [ $task == "hw" ]; then
        unset XCL_EMULATION_MODE
    else
        export XCL_EMULATION_MODE=$task
    fi

    echo ---------------------------------------------------------------------------
    echo 3.2 emulation
    ./$host $krnl.$task.${target_platform}.xclbin

    if [ $task == "sw_emu" ]; then
        mv ./profile_summary.csv ./"profile_summary.$task.csv"
        mv ./timeline_trace.csv ./"timeline_trace.$task.csv"
    elif [ $task == "sw_emu" ]; then
        mv ./profile_summary.csv ./"profile_summary.$task.csv"
        mv ./timeline_trace.csv ./"timeline_trace.$task.csv"
        mv ./profile_kernels.csv ./"profile_kernels.$task.csv"
        mv ./timeline_kernels.csv ./"timeline_kernels.$task.csv"
    fi
done # for

# To check the board status, run:
#  xbutil query
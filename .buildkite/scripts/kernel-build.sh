#!/bin/bash

echo ==============SCHEDULE: $SCHEDULE
echo ============$(pwd)

######## compile kernel
# echo ==================compile kernel
# make mrproper -j$(nproc)
# make O=./build ARCH=riscv openeuler_defconfig
# make O=./build ARCH=riscv -j$(nproc) Image modules dtbs
# make O=./build INSTALL_MOD_PATH=./install modules_install
# mkdir -p ./install/boot/dtb/thead
# cp ./build/arch/riscv/boot/Image ./install/boot/
# find ./build/arch/riscv/boot/dts/ -name *.dtb | xargs -i cp {} ./install/boot/dtb/
# cp ./install/boot/dtb/th*.dtb ./install/boot/dtb/thead
# tar -cvf riscv-$(date +"%Y%m%d%H%M").tar -C ./install .

######## kselftests
# echo ==================kselftests
# make distclean
# make O=./kselftest openeuler_defconfig
# make O=./kselftest headers -j$(nproc)
# make O=./kselftest -C tools/testing/selftests SKIP_TARGETS="hid bpf" -j$(nproc)
# make O=./kselftest SKIP_TARGETS="hid bpf" kselftest -j$(nproc)

####### smatch
echo ==================smatch
make distclean
SMATCH_PATH=/opt/smatch
$SMATCH_PATH/smatch_scripts/build_kernel_data.sh

if [ "x$SCHEDULE" == "x1" ]; then
    ####### smatch
    echo ----------smatch_scripts/test_kernel.sh
    make distclean
    $SMATCH_PATH/smatch_scripts/test_kernel.sh
fi
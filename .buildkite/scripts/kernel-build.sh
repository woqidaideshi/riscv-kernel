#!/bin/bash
make mrproper -j$(nproc)
make O=./build ARCH=riscv openeuler_defconfig
make O=./build ARCH=riscv -j$(nproc) Image modules dtbs
make O=./build INSTALL_MOD_PATH=./install modules_install
mkdir -p ./install/boot/dtb/thead
cp ./build/arch/riscv/boot/Image ./install/boot/
find ./build/arch/riscv/boot/dts/ -name *.dtb | xargs -i cp {} ./install/boot/dtb/
cp ./install/boot/dtb/th*.dtb ./install/boot/dtb/thead
tar -cvf riscv-$(date +"%Y%m%d%H%M").tar -C ./install .

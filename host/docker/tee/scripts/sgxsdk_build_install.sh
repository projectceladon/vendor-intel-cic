#!/bin/sh
set -ex

SGX_SRC_PATH="/tmp/linux-sgx/"
SGX_INSTALL_PATH="/opt/intel/"

# Compile SDK and install
cd $SGX_SRC_PATH
make sdk -j32
make sdk_install_pkg -j32
cd $SGX_INSTALL_PATH
cp $SGX_SRC_PATH/linux/installer/bin/sgx_linux_x64_sdk_*.bin ./
yes yes | ./sgx_linux_x64_sdk_*.bin

# Compile PSW and install
# Note that the compilation of PSW requires the installation of SDK.
cd $SGX_SRC_PATH
make psw -j32
make psw_install_pkg -j32
cd $SGX_INSTALL_PATH
cp $SGX_SRC_PATH/linux/installer/bin/sgx_linux_x64_psw_*.bin ./
./sgx_linux_x64_psw_*.bin --no-start-aesm
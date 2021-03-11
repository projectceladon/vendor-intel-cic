#!/bin/sh

set -ex
APP_NAME=cickeymasterd
APP_MANIFEST_SIG=$APP_NAME.sig
APP_MANIFEST_TOKEN=$APP_NAME.token

# start the aesmd service
if ! pgrep "aesm_service" > /dev/null ; then
    LD_LIBRARY_PATH="/opt/intel/sgxpsw/aesm:$LD_LIBRARY_PATH" /opt/intel/sgxpsw/aesm/aesm_service
fi

# Generate EINITOKEN for the target application
/graphene/signer/pal-sgx-get-token \
	-sig $APP_MANIFEST_SIG \
	-output $APP_MANIFEST_TOKEN

# Set app map addr, currently this's a workaround for graphene
sysctl vm.mmap_min_addr=0

# Run the App
SGX=1 /graphene/Runtime/pal-Linux-SGX init $APP_NAME
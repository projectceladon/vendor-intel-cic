#!/bin/bash

set -ex

APP_NAME=cickeymasterd
APP_MANIFEST=$APP_NAME.manifest
SIGNER_KEY="enclave-key.pem"

function log() {
    echo "[$0 ($$/$PPID)] $@"
}

function sign_manifests() {
	log "sign the manifests to get *.sig and *.manifests.sgx"
	/graphene/signer/pal-sgx-sign \
		-libpal /graphene/Runtime/libpal-Linux-SGX.so \
		-key /tee/tmp/$SIGNER_KEY \
		-manifest /tee/tmp/$APP_MANIFEST \
		-output $APP_MANIFEST.sgx \
		-exec $APP_NAME
}

sign_manifests

#!/bin/bash
set -ex

DOCKER_IMG=dev:latest
RUN_ARG="--network=host --env http_proxy="http://child-prc.intel.com:913" --env https_proxy="http://child-prc.intel.com:913""

docker run -it $RUN_ARG $DOCKER_IMG bash

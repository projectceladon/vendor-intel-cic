FROM graphene-runtime:latest as tee-intermediate

RUN mkdir /tee \
	&& mkdir -p /tee/tmp

COPY bin/cickeymasterd /tee

# Copy template scripts and manifests
COPY bin/enclave-key.pem /tee/tmp
COPY conf/cickeymasterd.manifest /tee/tmp
COPY scripts/argv_serializer.sh /tee/tmp
COPY scripts/sign_manifests.sh /tee/tmp

# Generate the trusted_argv
WORKDIR /tee
RUN chmod u+x tmp/argv_serializer.sh \
	&& ./tmp/argv_serializer.sh cickeymasterd

# Sign the final manifest file
RUN chmod u+x tmp/sign_manifests.sh \
	&& ./tmp/sign_manifests.sh \
	&& rm -rf /tee/tmp

# Create the final tee docker with multi-buildstage docker
# which could reduce the size of the docker image and also
# could assure not expose any secrets or sensitive data in
# the final docker image
FROM ubuntu:18.04 as tee

# Add steps here to set up dependencies
RUN env DEBIAN_FRONTEND=noninteractive apt-get update \
    && env DEBIAN_FRONTEND=noninteractive apt-get install -y \
		net-tools \
		build-essential \
		libssl-dev \
		libprotobuf-dev \
		libcurl4-openssl-dev \
		python \
		python3 \
		python3-protobuf \
		libprotobuf-c-dev \
	&& apt-get clean \
	&& rm -rf /var/lib/apt/lists/*

RUN mkdir -p /tee \
    && mkdir -p /graphene/Runtime \
    && mkdir -p /graphene/signer \
	&& mkdir -p /opt/intel

# Copy Graphene related files
COPY --from=tee-intermediate /graphene/Runtime/ /graphene/Runtime
COPY --from=tee-intermediate /graphene/signer/ /graphene/signer

# Copy SGX-SDK related files
COPY --from=tee-intermediate /opt/intel/sgx_linux_x64_sdk_*.bin /opt/intel
COPY --from=tee-intermediate /opt/intel/sgx_linux_x64_psw_*.bin /opt/intel

# Copy TEE related files
COPY --from=tee-intermediate /tee/ /tee

# Install SGX-SDK
WORKDIR /opt/intel
RUN yes yes | ./sgx_linux_x64_sdk_*.bin \
	&& ./sgx_linux_x64_psw_*.bin --no-start-aesm \
	&& rm -rf sgx_linux_x64*.bin \
	&& echo 'source /opt/intel/sgxsdk/environment' >> /root/.bashrc

# Copy apploader to /tee
WORKDIR /tee
COPY scripts/apploader.sh /tee
RUN chmod u+x apploader.sh

# Define default command
ENTRYPOINT ["/bin/sh", "./apploader.sh"]


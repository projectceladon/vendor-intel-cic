set(TARGET_OS "linux")
set(CMAKE_C_COMPILER "/usr/bin/clang")
set(CMAKE_CXX_COMPILER "/usr/bin/clang++")

cmake_minimum_required( VERSION 2.6 )
project(CIC-HOST-KEYMASTERD)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__ANDROID__=1 -DOS_UBUNTU -fPIC -std=gnu11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__ANDROID__=1 -DOS_UBUNTU -fPIC -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb -DHOST_KEYMASTER_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/build/bin/${TARGET_OS}")
set(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/build/libs/${TARGET_OS}")

set(AIDL_HOSTKEYMASTER_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/../aidl/android/hardware/hostkeymaster/IHostKeymasterd.aidl
  )

set(KEYMASTER_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../system/keymaster/" )
set(LIB_HARDWARE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../hardware/libhardware")
set(LIB_BORINGSSL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../external/boringssl")

find_library(PTHREAD_LIBRARY pthread "pthread library")

link_directories(
	"${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/${TARGET_OS}/libbinder/lib"
	"${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/${TARGET_OS}/libboringssl"
)

include_directories(
	"${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/${TARGET_OS}/libbinder/include"
	"${CMAKE_CURRENT_SOURCE_DIR}/include"
	"${CMAKE_CURRENT_SOURCE_DIR}/../aidl/include"
	"${LIB_HARDWARE_ROOT}/include"
	"${LIB_BORINGSSL_ROOT}/include"
	"${KEYMASTER_ROOT}/include"
)

macro (add_aidl_cpp _output_list)
  foreach(_aidl_file ${ARGN})
    get_filename_component(_out_file ${_aidl_file} NAME_WE)
    set(_out_cpp ${CMAKE_CURRENT_SOURCE_DIR}/${_out_file}.cpp)

    add_custom_command(
      OUTPUT ${_out_cpp}
      DEPENDS ${_aidl_file}
      COMMAND aidl-cpp -I. ${_aidl_file} include ${_out_cpp}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMENT "aidl-cpp -I. ${_aidl_file} include ${_out_cpp}"
    )
    set_property(SOURCE ${_out_cpp} APPEND PROPERTY OBJECT_DEPENDS ${_aidl_file})
    set_source_files_properties(${_out_cpp} PROPERTIES   GENERATED TRUE)
    set_source_files_properties(${_aidl_file} PROPERTIES HEADER_FILE_ONLY TRUE)
    set(${_output_list} ${${_output_list}}  ${_out_cpp})
  endforeach(_aidl_file ${ARGN})
endmacro (add_aidl_cpp)

add_aidl_cpp(AIDL_HOSTKEYMASTER_CPP ${AIDL_HOSTKEYMASTER_FILES})

set( LIB_KEYMASTER_SRC
	${KEYMASTER_ROOT}/android_keymaster/android_keymaster.cpp
	${KEYMASTER_ROOT}/android_keymaster/android_keymaster_messages.cpp
	${KEYMASTER_ROOT}/android_keymaster/android_keymaster_utils.cpp
	${KEYMASTER_ROOT}/android_keymaster/authorization_set.cpp
	${KEYMASTER_ROOT}/android_keymaster/keymaster_enforcement.cpp
	${KEYMASTER_ROOT}/android_keymaster/keymaster_tags.cpp
	${KEYMASTER_ROOT}/android_keymaster/operation.cpp
	${KEYMASTER_ROOT}/android_keymaster/operation_table.cpp
	${KEYMASTER_ROOT}/android_keymaster/serializable.cpp
	${KEYMASTER_ROOT}/android_keymaster/logger.cpp
	${KEYMASTER_ROOT}/key_blob_utils/auth_encrypted_key_blob.cpp
	${KEYMASTER_ROOT}/key_blob_utils/integrity_assured_key_blob.cpp
	${KEYMASTER_ROOT}/key_blob_utils/ocb.c
	${KEYMASTER_ROOT}/key_blob_utils/ocb_utils.cpp
	${KEYMASTER_ROOT}/key_blob_utils/software_keyblobs.cpp
	${KEYMASTER_ROOT}/km_openssl/aes_key.cpp
	${KEYMASTER_ROOT}/km_openssl/aes_operation.cpp
	${KEYMASTER_ROOT}/km_openssl/asymmetric_key.cpp
	${KEYMASTER_ROOT}/km_openssl/asymmetric_key_factory.cpp
	${KEYMASTER_ROOT}/km_openssl/attestation_record.cpp
	${KEYMASTER_ROOT}/km_openssl/block_cipher_operation.cpp
	${KEYMASTER_ROOT}/km_openssl/ckdf.cpp
	${KEYMASTER_ROOT}/km_openssl/ec_key.cpp
	${KEYMASTER_ROOT}/km_openssl/ec_key_factory.cpp
	${KEYMASTER_ROOT}/km_openssl/ecdsa_operation.cpp
	${KEYMASTER_ROOT}/km_openssl/ecies_kem.cpp
	${KEYMASTER_ROOT}/km_openssl/hkdf.cpp
	${KEYMASTER_ROOT}/km_openssl/hmac.cpp
	${KEYMASTER_ROOT}/km_openssl/hmac_key.cpp
	${KEYMASTER_ROOT}/km_openssl/hmac_operation.cpp
	${KEYMASTER_ROOT}/km_openssl/iso18033kdf.cpp
	${KEYMASTER_ROOT}/km_openssl/kdf.cpp
	${KEYMASTER_ROOT}/km_openssl/nist_curve_key_exchange.cpp
	${KEYMASTER_ROOT}/km_openssl/openssl_err.cpp
	${KEYMASTER_ROOT}/km_openssl/openssl_utils.cpp
	${KEYMASTER_ROOT}/km_openssl/rsa_key.cpp
	${KEYMASTER_ROOT}/km_openssl/rsa_key_factory.cpp
	${KEYMASTER_ROOT}/km_openssl/rsa_operation.cpp
	${KEYMASTER_ROOT}/km_openssl/software_random_source.cpp
	${KEYMASTER_ROOT}/km_openssl/symmetric_key.cpp
	${KEYMASTER_ROOT}/km_openssl/triple_des_key.cpp
	${KEYMASTER_ROOT}/km_openssl/triple_des_operation.cpp
	${KEYMASTER_ROOT}/km_openssl/wrapped_key.cpp
	)

add_library(libkeymaster STATIC ${LIB_KEYMASTER_SRC})

target_link_libraries(libkeymaster
	pthread
	boringssl
)

set( LIB_KEYMASTER_CONTEXT_SRC
	host_keymaster_context.cpp
	${KEYMASTER_ROOT}/android_keymaster/logger.cpp
    ${KEYMASTER_ROOT}/contexts/pure_soft_keymaster_context.cpp
    ${KEYMASTER_ROOT}/contexts/soft_attestation_cert.cpp
    ${KEYMASTER_ROOT}/km_openssl/attestation_utils.cpp
    ${KEYMASTER_ROOT}/km_openssl/soft_keymaster_enforcement.cpp
)

add_library(libkeymaster_context STATIC ${LIB_KEYMASTER_CONTEXT_SRC})

target_link_libraries(libkeymaster_context
	pthread
	boringssl
)
	
add_executable(hostkeymasterd
	main.cpp
	host_keymaster_ipc.cpp
	${AIDL_HOSTKEYMASTER_CPP}
)

target_link_libraries(hostkeymasterd
	binder
	libkeymaster_context
	libkeymaster
)


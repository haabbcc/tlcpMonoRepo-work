cmake_minimum_required(VERSION 3.10)
project(pqmagic_openssl_provider C)

get_filename_component(TLCP_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
set(_PQMAGIC_PREFIX "${TLCP_ROOT}/pqmagic/install")
set(_TONGSUO_ROOT "${TLCP_ROOT}/tongsuo")

set(OPENSSL_ROOT_DIR "${_TONGSUO_ROOT}")
set(OPENSSL_INCLUDE_DIR "${_TONGSUO_ROOT}/include")
set(OPENSSL_CRYPTO_LIBRARY "${_TONGSUO_ROOT}/libcrypto.so")

find_library(PQMAGIC_LIB pqmagic_std
  PATHS "${_PQMAGIC_PREFIX}/lib" /usr/local/lib
  NO_DEFAULT_PATH)

if(NOT PQMAGIC_LIB)
  find_library(PQMAGIC_LIB pqmagic_std)
endif()

if(NOT PQMAGIC_LIB)
  message(FATAL_ERROR "libpqmagic_std not found; 请先运行 build-all.sh 或手动构建 PQMagic 到 pqmagic/install")
endif()

add_library(pqmagic MODULE pqmagic_prov.c)
set_target_properties(pqmagic PROPERTIES PREFIX "" OUTPUT_NAME "pqmagic")

target_include_directories(pqmagic PRIVATE
  "${_PQMAGIC_PREFIX}/include"
  "${OPENSSL_INCLUDE_DIR}")
target_link_libraries(pqmagic PRIVATE "${OPENSSL_CRYPTO_LIBRARY}" ${PQMAGIC_LIB})

target_compile_options(pqmagic PRIVATE -Wall -fPIC)

set_target_properties(pqmagic PROPERTIES
  BUILD_RPATH "${_TONGSUO_ROOT};${_PQMAGIC_PREFIX}/lib"
)

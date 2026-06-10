cmake_minimum_required(VERSION 3.10)
project(aigis_enc_openssl_provider C)

get_filename_component(TLCP_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
set(_OPENSSL_ROOT "${TLCP_ROOT}/tongsuo")
set(_PQMAGIC_PREFIX "${TLCP_ROOT}/pqmagic/install")

set(OPENSSL_INCLUDE_DIR "${_OPENSSL_ROOT}/include")
set(OPENSSL_CRYPTO_LIBRARY "${_OPENSSL_ROOT}/libcrypto.so")

find_library(PQMAGIC_LIB pqmagic_std PATHS "${_PQMAGIC_PREFIX}/lib" NO_DEFAULT_PATH)
if(NOT PQMAGIC_LIB)
  find_library(PQMAGIC_LIB pqmagic_std)
endif()
if(NOT PQMAGIC_LIB)
  message(FATAL_ERROR "libpqmagic_std not found; 请先运行 build-all.sh")
endif()

add_library(aigis_enc MODULE src/aigis_enc_prov.c)
set_target_properties(aigis_enc PROPERTIES PREFIX "" OUTPUT_NAME "aigis_enc")

target_include_directories(aigis_enc PRIVATE "${_PQMAGIC_PREFIX}/include" "${OPENSSL_INCLUDE_DIR}")
target_link_libraries(aigis_enc PRIVATE "${OPENSSL_CRYPTO_LIBRARY}" ${PQMAGIC_LIB})
target_compile_options(aigis_enc PRIVATE -Wall -Wextra -fPIC)

set_target_properties(aigis_enc PROPERTIES
  BUILD_RPATH "${_OPENSSL_ROOT};${_PQMAGIC_PREFIX}/lib"
  INSTALL_RPATH "${_PQMAGIC_PREFIX}/lib"
)

add_executable(test_aigis_kem tools/test_aigis_kem.c)
target_include_directories(test_aigis_kem PRIVATE "${OPENSSL_INCLUDE_DIR}")
target_link_libraries(test_aigis_kem PRIVATE "${OPENSSL_CRYPTO_LIBRARY}")
set_target_properties(test_aigis_kem PROPERTIES
  BUILD_RPATH "${_OPENSSL_ROOT};${_PQMAGIC_PREFIX}/lib"
)
target_compile_definitions(test_aigis_kem PRIVATE
  AIGIS_ENC_PROVIDER_SO="$<TARGET_FILE:aigis_enc>"
)
add_dependencies(test_aigis_kem aigis_enc)

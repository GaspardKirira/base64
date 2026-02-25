cmake_minimum_required(VERSION 3.20)
project(base64 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Vix deps (provides target: vixcpp::vix_ffi)
include(.vix/vix_deps.cmake)

# ---------------------------
# Header-only library target
# ---------------------------
add_library(base64 INTERFACE)
add_library(base64::base64 ALIAS base64)

target_include_directories(base64 INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

if (MSVC)
  target_compile_options(base64 INTERFACE /W4 /permissive-)
else()
  target_compile_options(base64 INTERFACE -Wall -Wextra -Wpedantic)
endif()

# Optional: expose vix-ffi headers to consumers of base64 (harmless, but clean)
target_link_libraries(base64 INTERFACE vixcpp::vix_ffi)

# ---------------------------
# Optional FFI shared library
# ---------------------------
option(BASE64_BUILD_FFI "Build base64 FFI shared library" ON)

if (BASE64_BUILD_FFI)
  set(VIX_FFI_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.vix/deps/vixcpp.vix_ffi")
  if (NOT EXISTS "${VIX_FFI_ROOT}/cmake/VixFFI.cmake")
    message(FATAL_ERROR
      "vix-ffi not found at ${VIX_FFI_ROOT}. Run: vix add vixcpp/vix_ffi@0.1.0 && vix deps"
    )
  endif()
  include("${VIX_FFI_ROOT}/cmake/VixFFI.cmake")

  set(BASE64_FFI_SRC "${CMAKE_CURRENT_SOURCE_DIR}/ffi/src/base64_ffi.cpp")
  if (NOT EXISTS "${BASE64_FFI_SRC}")
    message(FATAL_ERROR
      "FFI source not found: ${BASE64_FFI_SRC}\n"
      "Expected layout: ffi/src/base64_ffi.cpp\n"
      "Either create it, or disable FFI: -DBASE64_BUILD_FFI=OFF"
    )
  endif()

  vix_add_ffi_library(
    NAME base64_ffi
    OUTPUT_NAME base64
    SOURCES
      "${BASE64_FFI_SRC}"
    PUBLIC_INCLUDES
      ${CMAKE_CURRENT_SOURCE_DIR}/ffi/include
      ${CMAKE_CURRENT_SOURCE_DIR}/include
    PUBLIC_LIBS
      base64::base64
      vixcpp::vix_ffi
    VERSION 1.0.0
    SOVERSION 1
  )

  vix_install_ffi_library(base64_ffi)
  install(DIRECTORY ffi/include/ DESTINATION include)
endif()

# ---------------------------
# Tests
# ---------------------------
include(CTest)
if (BUILD_TESTING)
  add_executable(base64_basic_test tests/test_basic.cpp)
  target_link_libraries(base64_basic_test PRIVATE base64::base64)
  add_test(NAME base64.basic COMMAND base64_basic_test)
endif()

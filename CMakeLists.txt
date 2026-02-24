cmake_minimum_required(VERSION 3.20)
project(base64 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

include(CTest)
enable_testing()

add_executable(base64_basic_test tests/test_basic.cpp)
target_link_libraries(base64_basic_test PRIVATE base64::base64)
add_test(NAME base64.basic COMMAND base64_basic_test)

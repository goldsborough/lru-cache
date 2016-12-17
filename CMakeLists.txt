###########################################################
## CMAKE SETUP
###########################################################

cmake_minimum_required(VERSION 3.2)
project(lru-cache)

########################################
# C++ VERSIONING
########################################

include(CheckCXXCompilerFlag)

check_cxx_compiler_flag("-std=c++14" COMPILER_SUPPORTS_CXX_14)
check_cxx_compiler_flag("-std=c++1z" COMPILER_SUPPORTS_CXX_1z)
check_cxx_compiler_flag("-std=c++17" COMPILER_SUPPORTS_CXX_17)

if (COMPILER_SUPPORTS_CXX_17)
    message(STATUS "Compiling with C++17")
    set(CMAKE_CXX_STANDARD 17)
elseif (COMPILER_SUPPORTS_CXX_1z)
  message(STATUS "Compiling with C++1z")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z")
elseif (COMPILER_SUPPORTS_CXX_14)
  message(STATUS "Compiling with C++14")
    set(CMAKE_CXX_STANDARD 14)
else()
    message(FATAL_ERROR "Please install a modern C++ compiler, they are not expensive.")
endif()


###########################################################
## INCLUDES
###########################################################

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

###########################################################
## SOURCES
###################################################FS########

set(LRU_CACHE_SOURCES main.cpp)

###########################################################
## TARGETS
###########################################################

set(LRU_CACHE_BINARY_DIR ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${LRU_CACHE_BINARY_DIR})

add_executable(lru-cache ${LRU_CACHE_SOURCES})

###########################################################
## COMPILER FLAGS
###########################################################

target_compile_options(lru-cache PUBLIC -Os -O2 -g -Werror)

########################################
# TESTS
########################################

option(BUILD_LRU_CACHE_TESTS "Enable tests" ON)

if(BUILD_LRU_CACHE_TESTS)
  message(STATUS "Enabling and adding tests ...")
  enable_testing()
  add_subdirectory(tests)
else()
  message(STATUS "Disabling and not building tests ...")
endif()

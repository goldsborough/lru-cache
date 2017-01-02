###########################################################
## CMAKE SETUP
###########################################################

cmake_minimum_required(VERSION 3.2)
project(lru-cache)

add_compile_options(-g)

########################################
# C++ VERSIONING
########################################

include(CheckCXXCompilerFlag)

check_cxx_compiler_flag("-std=c++14" COMPILER_SUPPORTS_CXX_14)
check_cxx_compiler_flag("-std=c++1z" COMPILER_SUPPORTS_CXX_1z)
check_cxx_compiler_flag("-std=c++17" COMPILER_SUPPORTS_CXX_17)

if (COMPILER_SUPPORTS_CXX_1z)
  message(STATUS "Compiling with C++1z")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z")
elseif (COMPILER_SUPPORTS_CXX_14)
  message(STATUS "Compiling with C++14")
    set(CMAKE_CXX_STANDARD 14)
else()
    message(FATAL_ERROR "Please install a modern C++ compiler, they are not expensive.")
endif()

###########################################################
## DEPENDENCIES
###########################################################

set(CMAKE_MODULE_PATH
  ${CMAKE_MODULE_PATH}
  "${CMAKE_SOURCE_DIR}/cmake/Modules/"
)

###########################################################
## INCLUDES
###########################################################

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

###########################################################
## BINARIES
###########################################################

set(LRU_CACHE_BINARY_DIR ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${LRU_CACHE_BINARY_DIR})

########################################
# TESTS
########################################

option(BUILD_LRU_CACHE_TESTS "Enable tests" ON)

if(BUILD_LRU_CACHE_TESTS)
  message(STATUS "Enabling tests ...")
  enable_testing()
  add_subdirectory(tests)
else()
  message(STATUS "Disabling tests ...")
endif()

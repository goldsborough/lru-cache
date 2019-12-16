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

# Need this top-level include for "tests/"
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

###########################################################
## EXAMPLES
###########################################################

add_subdirectory(examples)

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


add_library(lru INTERFACE)
target_include_directories(lru INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        )

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
        "${PROJECT_BINARY_DIR}/lruConfigVersion.cmake"
        VERSION 0.1
        COMPATIBILITY AnyNewerVersion
)

install(TARGETS lru
        EXPORT lruTargets
        LIBRARY DESTINATION lib COMPONENT Runtime
        ARCHIVE DESTINATION lib COMPONENT Development
        RUNTIME DESTINATION bin COMPONENT Runtime
        PUBLIC_HEADER DESTINATION include COMPONENT Development
        BUNDLE DESTINATION bin COMPONENT Runtime
        )

include(CMakePackageConfigHelpers)
configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/lruConfig.cmake.in"
        "${PROJECT_BINARY_DIR}/lruConfig.cmake"
        INSTALL_DESTINATION lib/cmake/lru
)

install(EXPORT lruTargets DESTINATION lib/cmake/lru)
install(FILES "${PROJECT_BINARY_DIR}/lruConfigVersion.cmake"
        "${PROJECT_BINARY_DIR}/lruConfig.cmake"
        DESTINATION lib/cmake/lru)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/ DESTINATION include)

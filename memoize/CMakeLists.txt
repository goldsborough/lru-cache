###########################################################
## SOURCES
###########################################################

set(MEMOIZE_SOURCES
  memoize.cpp
  action.cpp
  consumer.cpp
  handler.cpp
)

###########################################################
## DEPENDENCIES
###########################################################

set(CLANG_LIBS
  clangAST
  clangASTMatchers
  clangAnalysis
  clangBasic
  clangDriver
  clangEdit
  clangFrontend
  clangFrontendTool
  clangLex
  clangParse
  clangSema
  clangEdit
  clangRewrite
  clangRewriteFrontend
  clangStaticAnalyzerFrontend
  clangStaticAnalyzerCheckers
  clangStaticAnalyzerCore
  clangSerialization
  clangToolingCore
  clangTooling
  clangFormat
)

# Uses root/cmake/modules/FindLLVM.cmake
find_package(LLVM 3.9 REQUIRED)

include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})

###########################################################
## TARGETS
###########################################################

add_executable(memoize-tool ${MEMOIZE_SOURCES})

target_include_directories(memoize-tool PUBLIC ${LLVM_INCLUDE_DIR})
target_link_libraries(memoize-tool
  ${CLANG_LIBS}
  ${LLVM_LIBRARIES}
  ${LLVM_LDFLAGS}
)

###########################################################
## COMPILER FLAGS
###########################################################

separate_arguments(LLVM_CXXFLAGS)
list(REMOVE_ITEM LLVM_CXXFLAGS "-std=c++0x" "-std=c++11" "-std=c++14" "-std=c++1z")

target_compile_options(memoize-tool PUBLIC -Os -O2 -Werror ${LLVM_CXXFLAGS})

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

###########################################################
## TARGETS
###########################################################

add_executable(memoize-tool ${MEMOIZE_SOURCES})

target_link_libraries(memoize-tool ${CLANG_LIBS})

###########################################################
## COMPILER FLAGS
###########################################################

target_compile_options(memoize-tool PUBLIC -Os -O2 -Werror  $(llvm-config --cxxflags) $(llvm-config --libs --ldflags --system-libs))

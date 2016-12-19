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

set(LLVM_LIBS
  LLVMLTO LLVMObjCARCOpts LLVMSymbolize LLVMDebugInfoPDB LLVMDebugInfoDWARF LLVMGlobalISel LLVMCoverage LLVMTableGen LLVMOrcJIT LLVMMIRParser LLVMNVPTXCodeGen LLVMNVPTXDesc LLVMNVPTXInfo LLVMNVPTXAsmPrinter LLVMARMDisassembler LLVMARMCodeGen LLVMARMAsmParser LLVMARMDesc LLVMARMInfo LLVMARMAsmPrinter LLVMAMDGPUDisassembler LLVMAMDGPUCodeGen LLVMAMDGPUAsmParser LLVMAMDGPUDesc LLVMAMDGPUInfo LLVMAMDGPUAsmPrinter LLVMAMDGPUUtils LLVMObjectYAML LLVMX86Disassembler LLVMX86AsmParser LLVMX86CodeGen LLVMSelectionDAG LLVMAsmPrinter LLVMDebugInfoCodeView LLVMX86Desc LLVMMCDisassembler LLVMX86Info LLVMX86AsmPrinter LLVMX86Utils LLVMMCJIT LLVMLibDriver LLVMOption LLVMLineEditor LLVMPasses LLVMipo LLVMVectorize LLVMLinker LLVMIRReader LLVMAsmParser LLVMInterpreter LLVMExecutionEngine LLVMRuntimeDyld LLVMObject LLVMMCParser LLVMCodeGen LLVMTarget LLVMScalarOpts LLVMInstCombine LLVMInstrumentation LLVMTransformUtils LLVMMC LLVMBitWriter LLVMBitReader LLVMAnalysis LLVMProfileData LLVMCore LLVMSupport
  curses z m
)

link_directories(/usr/local/Cellar/llvm/3.9.0/lib)


###########################################################
## TARGETS
###########################################################

add_executable(memoize-tool ${MEMOIZE_SOURCES})

target_link_libraries(memoize-tool ${CLANG_LIBS} ${LLVM_LIBS})

###########################################################
## COMPILER FLAGS
###########################################################

target_compile_options(memoize-tool PUBLIC
  -Os
  -O2
  -Werror
-I/usr/local/Cellar/llvm/3.9.0/include  -stdlib=libc++ -fPIC -fvisibility-inlines-hidden -Wall -W -Wno-unused-parameter -Wwrite-strings -Wcast-qual -Wmissing-field-initializers -pedantic -Wcovered-switch-default -Wnon-virtual-dtor -Wdelete-non-virtual-dtor -Werror=date-time -DNDEBUG   -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS)

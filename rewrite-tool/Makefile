
CLANG_LIBS := \
	-lLLVM \
	-lclangAST \
	-lclangASTMatchers \
	-lclangAnalysis \
	-lclangBasic \
	-lclangDriver \
	-lclangEdit \
	-lclangFrontend \
	-lclangFrontendTool \
	-lclangLex \
	-lclangParse \
	-lclangSema \
	-lclangEdit \
	-lclangRewrite \
	-lclangRewriteFrontend \
	-lclangStaticAnalyzerFrontend \
	-lclangStaticAnalyzerCheckers \
	-lclangStaticAnalyzerCore \
	-lclangSerialization \
	-lclangToolingCore \
	-lclangTooling \
	-lclangFormat \

all: tool test_memoized.cpp

tool: tool.cpp
	clang++-3.8 `llvm-config-3.8 --cxxflags` -std=c++14 -Wall -O2 -g -o tool tool.cpp `llvm-config-3.8 --ldflags` -Wl,--start-group ${CLANG_LIBS} -Wl,--end-group

test_memoized.cpp: tool test.cpp
	./tool test.cpp -- -std=c++14

.phony: clean
clean:
	rm tool test_memoized.cpp

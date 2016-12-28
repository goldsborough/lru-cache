/// The MIT License (MIT)
/// Copyright (c) 2016 Peter Goldsborough and Markus Engel
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to
/// deal in the Software without restriction, including without limitation the
/// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
/// sell copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.

#include <memory>

#include "clang/Frontend/CompilerInstance.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/ADT/StringRef.h"

#include "memoize/action.hpp"
#include "memoize/consumer.hpp"

namespace Memoize {
Action::ASTConsumerPointer
Action::CreateASTConsumer(clang::CompilerInstance& Compiler, llvm::StringRef) {
  // The rewriter will allow us to perform modifications to the AST's source.
  Rewriter.setSourceMgr(Compiler.getSourceManager(), Compiler.getLangOpts());

  return std::make_unique<Memoize::Consumer>(Rewriter);
}

bool Action::BeginSourceFileAction(clang::CompilerInstance& Compiler,
                                   llvm::StringRef Filename) {
  llvm::errs() << "Processing file '" << Filename << "' ...\n";

  // Success
  return true;
}

void Action::EndSourceFileAction() {
  // Simply stream the result of processing this source file to STDOUT
  // The user can always pipe the contents into an output file him/herself.
  const auto file = Rewriter.getSourceMgr().getMainFileID();
  Rewriter.getEditBuffer(file).write(llvm::outs());
}
}  // namespace Memoize

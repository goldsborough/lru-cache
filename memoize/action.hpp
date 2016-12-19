/**
* The MIT License (MIT)
* Copyright (c) 2016 Peter Goldsborough and Markus Engel
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#include <memory>

#include "clang/AST/ASTConsumer.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/ADT/StringRef.h"

#include "memoize/consumer.hpp"

namespace Memoize {

/// The primary entry point to the memoization attribute.
///
/// In clang's infrastructure, a `FrontendAction` is any operation that can be
/// performed by the frontend. As such, a frontend action is supplied with the
/// compiler instance and fed each translation unit (input source file) and may
/// perform actions before, during or after processing of a source file. In our
/// case, the action is to create an `ASTConsumer`, which will then take an
/// abstract syntax tree and process it.
///
/// \see Memoize::Consumer
class Action : public clang::ASTFrontendAction {
public:
  using ASTConsumerPointer = std::unique_ptr<clang::ASTConsumer>;

  ASTConsumerPointer CreateASTConsumer(clang::CompilerInstance& Compiler,
                                       llvm::StringRef) override {
    // The rewriter will allow us to perform modifications to the AST's source.
    Rewriter.setSourceMgr(Compiler.getSourceManager(), Compiler.getLangOpts());

    return std::make_unique<Memoize::Consumer>(Rewriter);
  }

  bool BeginSourceFileAction(clang::CompilerInstance& Compiler,
                             llvm::StringRef Filename) override {
    llvm::outs() << "Processing file '" << Filename << "' ...\n";

    // Success
    return true;
  }

  void EndSourceFileAction() override {
    // Simply stream the result of processing this source file to STDOUT
    // The user can always pipe the contents into an output file him/herself.
    const auto file = Rewriter.getSourceMgr().getMainFileID();
    Rewriter.getEditBuffer(file).write(llvm::outs());
  }

private:
  /// The `Rewriter` instance we use to rewrite the AST.
  clang::Rewriter Rewriter;
};
}  // namespace Memoize

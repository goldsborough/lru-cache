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

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Frontend/ASTConsumers.h"

#include "memoize/handler.hpp"

namespace Memoize {

/// Consumes an AST and applies the match handler for appropriate functions.
///
/// The Consumer dispatches a `MatchFinder` with an AST match expression that
/// matches all functions annotated with a `memoize` attribute, which then
/// calls our natch handler for each match found.
class Consumer : public clang::ASTConsumer {
public:
  /// Constructs a new Consumer.
  ///
  /// \param Rewriter A rewriter instance to perform source modifications.
  explicit Consumer(clang::Rewriter& Rewriter) : Handler(Rewriter);

  /// Consumes the AST after a single translation unit has been parsed.
  ///
  /// \param Context The `ASTContext` for the parsed AST.
  void HandleTranslationUnit(clang::ASTContext& Context) override;

private:
  using clang::ast_matchers::MatchFinder;

  /// The MatchFinder instance to look for matching functions.
  MatchFinder MatchFinder;

  /// The match handler for annotated functions. It must
  /// survive the lifetime of the `MatchFinder`.
  MemoizeHandler Handler;
};

}  // namespace Memoize

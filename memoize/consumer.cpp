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

#include "memoize/annotation-matcher.hpp"
#include "memoize/consumer.hpp"

namespace Memoize {

Consumer::Consumer(clang::Rewriter& Rewriter) : Handler(Rewriter) {
  using namespace clang::ast_matchers;  // NOLINT(build/namespaces)

  // Matches all functions annotated with "memoize" and that are
  // *definitions* and not *declarations* (i.e. have a body).
  // clang-format off
    auto MemoizedMatcher =
        functionDecl(
          isAnnotatedWith("memoize"),
          hasBody(compoundStmt())
        ).bind("target");
  // clang-format on

  MatchFinder.addMatcher(MemoizedMatcher, &Handler);
}

void Consumer::HandleTranslationUnit(clang::ASTContext& Context) {
  MatchFinder.matchAST(Context);
}

}  // namespace Memoize

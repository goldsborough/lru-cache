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

#include <sstream>
#include <string>
#include <vector>

#include "clang/Lex/Lexer.h"
#include "clang/Rewrite/Core/Rewriter.h"

namespace Memoize {

class MemoizeHandler : public clang::ast_matchers::MatchFinder::MatchCallback {
public:
  explicit MemoizeHandler(clang::Rewriter& Rewriter) : Rewriter{Rewriter} {}

  // Converts a FunctionDecl object into a string representation of the
  // prototype of the function, optionally renaming it.
  std::string funcdecl2prototype(clang::FunctionDecl const* fd,
                                 clang::SourceManager* sm,
                                 clang::LangOptions lopt,
                                 std::string newName = "") {
    std::ostringstream result;

    result << fd->getReturnType().getAsString() << " ";

    if (newName.empty()) {
      result << fd->getNameAsString();
    } else {
      result << newName;
    }

    clang::SourceLocation b(
        fd->getLocation().getLocWithOffset(fd->getNameAsString().length()));
    clang::SourceLocation _e(fd->getBody()->getLocStart().getLocWithOffset(-1));
    clang::SourceLocation e(
        clang::Lexer::getLocForEndOfToken(_e, 0, *sm, lopt));

    result << std::string(sm->getCharacterData(b),
                          sm->getCharacterData(e) - sm->getCharacterData(b));

    return result.str();
  }

  void
  run(const clang::ast_matchers::MatchFinder::MatchResult& Result) override {
    clang::SourceManager* sm = Result.SourceManager;
    clang::LangOptions lopt = Result.Context->getLangOpts();

    // The function whose results should be memoized.
    auto const* mf =
        Result.Nodes.getNodeAs<clang::FunctionDecl>("memoized_function");

    // The actual function call.
    auto const* fc = Result.Nodes.getNodeAs<clang::CallExpr>("function_call");

    if (mf) {
      auto it = std::find(std::begin(memoizedFunctions),
                          std::end(memoizedFunctions), mf);

      // A function has to be wrapped only once of couse, no matter how often it
      // is called.
      if (it == std::end(memoizedFunctions)) {
        memoizedFunctions.emplace_back(mf);

        auto before = mf->getLocStart();
        auto after = mf->getLocEnd().getLocWithOffset(1);

        std::ostringstream beforess, afterss;

        // Block to be inserted before the function definition.
        beforess << "\n"
                 << "#include \"lru/lru.hpp\"\n"
                 << funcdecl2prototype(mf, sm, lopt, mf->getNameAsString() +
                                                         std::string("_cached"))
                 << ";"
                 << "\n\n";

        // Block to be inserted after the function definition.
        afterss << "\n\n"
                << funcdecl2prototype(mf, sm, lopt, mf->getNameAsString() +
                                                        std::string("_cached"))
                << " {\n"
                << "static auto cache = LRU::memoize<decltype("
                << mf->getNameAsString() << ")>(" << mf->getNameAsString()
                << ");\n"
                << "return cache(";

        // Get the names of all the parameters and create the argument list
        // inside of the braces.
        // Horrible.
        std::vector<std::string> params;

        std::transform(
            mf->param_begin(), mf->param_end(), std::back_inserter(params),
            [](clang::ParmVarDecl* p) { return p->getNameAsString(); });

        bool first = true;
        for (auto it = std::begin(params); it != std::end(params); ++it) {
          if (!first) {
            afterss << ", ";
          }

          first = false;

          afterss << *it;
        }

        afterss << ");\n"
                << "}\n\n";

        Rewriter.InsertTextBefore(before, beforess.str());
        Rewriter.InsertTextAfter(after, afterss.str());
      }
    }

    if (fc) {
      // functionCalls.emplace_back(fc);
      Rewriter.ReplaceText(fc->getLocStart(), mf->getNameAsString().length(),
                           mf->getNameAsString() + std::string("_cached"));
    }
  }

private:
  clang::Rewriter& Rewriter;

  std::vector<clang::FunctionDecl const*> memoizedFunctions;
};

}  // namespace Memoize

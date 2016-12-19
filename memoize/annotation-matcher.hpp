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

#include <string>

#include "clang/ASTMatchers/ASTMatchersInternal.h"
#include "clang/ASTMatchers/ASTMatchersMacros.h"

namespace clang {
namespace ast_matchers {

/// \brief Matches declaration that has a particular annotate attribute.
///
/// Given an annotation on a function, for example:
/// \code
///   __attribute__((annotate("myAnnotation"))) void f() { ... }
/// \endcode
/// the matcher `decl(isAnnotatedWith("myAnnotation"))` will match the function
/// declaration of `f`.
///
///
/// \see https://goo.gl/NggSGX
///
AST_MATCHER_P(Decl, isAnnotatedWith, std::string, TargetAnnotation) {
  for (const auto* Attribute : Node.attrs()) {
    if (auto Annotation = llvm::dyn_cast<AnnotateAttr>(Attribute)) {
      return Annotation->getAnnotation() == TargetAnnotation;
    }
    return false;
  }

  return false;
}

}  // namespace ast_matchers
}  // namespace clang

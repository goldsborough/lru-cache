#include <algorithm>
#include <memory>
#include <sstream>
#include <vector>

#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Lex/Lexer.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Rewrite/Core/Rewriter.h"

static llvm::cl::OptionCategory MemoizableRewriterCategory{"Memoizable Rewriter"};


namespace clang {
namespace ast_matchers {
/// \brief Matches declaration that has an annotate attribute with a certain argument.
///
/// Given
/// \code
///   __attribute__((annotate("myAnnotation"))) void f() { ... }
/// \endcode
/// decl(isAnnotatedWith("myAnnotation")) matches the function declaration of f.
AST_MATCHER_P(Decl, isAnnotatedWith, std::string, str) {
    for (auto const* Attr : Node.attrs()) {
        if (Attr->getKind() == attr::Annotate && cast<AnnotateAttr>(Attr)->getAnnotation() == str) {
            return true;
        }
    }
        
    return false;
}
}
}


namespace MemoizableRewriter {
    using namespace clang;
    using namespace clang::ast_matchers;
    
    // All function calls whose callee is a function which has got an annotate("memoizable") attribute.
    // Get both the function and the function call.
    auto memoizedCallMatcher =
        callExpr(
            callee(
                functionDecl(
                    isAnnotatedWith("memoizable")
                ).bind("memoized_function")
            )
        ).bind("function_call")
    ;
    
    class MemoizableHandler : public MatchFinder::MatchCallback {
    public:
        MemoizableHandler(Rewriter& rewriter)
            : rewriter{rewriter}
        {
        }
        
        // Converts a FunctionDecl object into a string representation of the prototype of the function, optionally renaming it.
        std::string funcdecl2prototype(FunctionDecl const* fd, SourceManager* sm, LangOptions lopt, std::string newName = "") {
            std::ostringstream result;
            
            result << fd->getReturnType().getAsString() << " ";
            
            if (newName.empty()) {
                result << fd->getNameAsString();
            } else {
                result << newName;
            }
            
            SourceLocation b(fd->getLocation().getLocWithOffset(fd->getNameAsString().length()));
            SourceLocation _e(fd->getBody()->getLocStart().getLocWithOffset(-1));
            SourceLocation e(Lexer::getLocForEndOfToken(_e, 0, *sm, lopt));

            result << std::string(sm->getCharacterData(b), sm->getCharacterData(e) - sm->getCharacterData(b));
            
            return result.str();
        }
        
        virtual void run(const MatchFinder::MatchResult& Result) override {
            SourceManager* sm = Result.SourceManager;
            LangOptions lopt = Result.Context->getLangOpts();
            
            // The function whose results should be memoized.
            auto const* mf = Result.Nodes.getNodeAs<FunctionDecl>("memoized_function");
            
            // The actual function call.
            auto const* fc = Result.Nodes.getNodeAs<CallExpr>("function_call");
            

            if (mf) {
                auto it = std::find(std::begin(memoizedFunctions), std::end(memoizedFunctions), mf);

                // A function has to be wrapped only once of couse, no matter how often it is called.
                if (it == std::end(memoizedFunctions)) {
                    memoizedFunctions.emplace_back(mf);
                    
                    auto before = mf->getLocStart();
                    auto after = mf->getLocEnd().getLocWithOffset(1);
                    
                    std::ostringstream beforess, afterss;

                    // Block to be inserted before the function definition.
                    beforess
                        << "\n"
                        << "#include \"lru/lru.hpp\"\n"
                        << funcdecl2prototype(mf, sm, lopt, mf->getNameAsString() + std::string("_cached")) << ";"
                        << "\n\n";

                    // Block to be inserted after the function definition.
                    afterss
                        << "\n\n"
                        << funcdecl2prototype(mf, sm, lopt, mf->getNameAsString() + std::string("_cached")) << " {\n"
                        << "static auto cache = LRU::memoize<decltype(" << mf->getNameAsString() << ")>(" << mf->getNameAsString() << ");\n"
                        << "return cache(";

                    // Get the names of all the parameters and create the argument list inside of the braces.
                    // Horrible.
                    std::vector<std::string> params;

                    std::transform(mf->param_begin(), mf->param_end(), std::back_inserter(params), [](ParmVarDecl* p) { return p->getNameAsString(); });
                    
                    bool first = true;
                    for (auto it = std::begin(params); it != std::end(params); ++it) {
                        if (!first) {
                            afterss << ", ";
                        }
                        
                        first = false;
                        
                        afterss << *it;
                    }
                    
                    afterss
                        << ");\n"
                        << "}\n\n";
                        
                        
                    rewriter.InsertTextBefore(before, beforess.str());
                    rewriter.InsertTextAfter(after, afterss.str());
                }
            }
            
            if (fc) {
                //functionCalls.emplace_back(fc);
                rewriter.ReplaceText(fc->getLocStart(), mf->getNameAsString().length(), mf->getNameAsString() + std::string("_cached"));
            }
        }
        
    private:
        Rewriter& rewriter;

        std::vector<FunctionDecl const*> memoizedFunctions;
        //std::vector<CallExpr const*> functionCalls;
    };

    class MainConsumer : public clang::ASTConsumer {
    public:
        MainConsumer(clang::Rewriter& rewriter) 
            : handler{rewriter}
        {
            matchFinder.addMatcher(memoizedCallMatcher, &handler);
        }
        
        virtual void HandleTranslationUnit(clang::ASTContext& Ctx) override {
            matchFinder.matchAST(Ctx);
        }
        
    private:
        MatchFinder matchFinder;
        MemoizableHandler handler;
    };

    class MainAction : public clang::ASTFrontendAction {
    public:
        virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance& CI, llvm::StringRef InFile) override {
            rewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
            
            return std::make_unique<MainConsumer>(rewriter);
        }

        virtual void EndSourceFileAction() override {
            // How can the target file to write to be specified? Ask the CommonOptionsParser below?
            rewriter.getEditBuffer(rewriter.getSourceMgr().getMainFileID()).write(llvm::outs());        
        }
        
    private:
        Rewriter rewriter;
    };
}

int main(int argc, char const* argv[]) {
    using namespace clang::tooling;
    
    CommonOptionsParser cop(argc, argv, MemoizableRewriterCategory);
    ClangTool tool(cop.getCompilations(), cop.getSourcePathList());
    
    return tool.run(newFrontendActionFactory<MemoizableRewriter::MainAction>().get());
}

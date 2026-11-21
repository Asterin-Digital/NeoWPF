#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclCXX.h>
#include <clang/AST/Expr.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/TypeLoc.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Basic/IdentifierTable.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Lex/Lexer.h>
#include <clang/Lex/PPCallbacks.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Tooling/Core/Replacement.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/raw_ostream.h>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

namespace {

namespace fs = std::filesystem;
using clang::ast_matchers::MatchFinder;

llvm::cl::OptionCategory ToolCategory("symbol-renamer options");
llvm::cl::opt<std::string> OldName(
    "old", llvm::cl::desc("Symbol name to rename (qualified names supported)"),
    llvm::cl::Required, llvm::cl::cat(ToolCategory));
llvm::cl::opt<std::string> NewName(
    "new", llvm::cl::desc("New unqualified C++ identifier"),
    llvm::cl::Required, llvm::cl::cat(ToolCategory));
llvm::cl::opt<std::string> BuildPath(
    "p,build-path",
    llvm::cl::desc("Compilation database directory (default: current directory)"),
    llvm::cl::init("."), llvm::cl::cat(ToolCategory));
llvm::cl::opt<std::string> RootPath(
    "root", llvm::cl::desc("Restrict source discovery and edits to this folder"),
    llvm::cl::cat(ToolCategory));
llvm::cl::opt<bool> DryRun(
    "dry-run", llvm::cl::desc("Show edits without writing files"),
    llvm::cl::init(false), llvm::cl::cat(ToolCategory));
llvm::cl::list<std::string> Inputs(
    llvm::cl::Positional, llvm::cl::desc("[source file or folder ...]"),
    llvm::cl::ZeroOrMore, llvm::cl::cat(ToolCategory));

struct SymbolKey {
    std::string value;

    bool operator<(const SymbolKey& other) const { return value < other.value; }
};

struct Edit {
    std::string path;
    unsigned offset;
    unsigned length;
    unsigned line;
    unsigned column;
    std::string replacement;
};

struct RunState {
    std::string oldName;
    std::string newName;
    std::vector<fs::path> roots;
    std::set<std::string> selectedFiles;
    std::set<SymbolKey> candidates;
    std::map<SymbolKey, std::vector<Edit>> edits;
    std::vector<Edit> includeEdits;
    std::set<std::pair<std::string, unsigned>> editLocations;
    bool hadEditError = false;
};

std::string normalizedPath(const fs::path& path) {
    std::error_code error;
    fs::path result = fs::weakly_canonical(path, error);
    if (error) {
        error.clear();
        result = fs::absolute(path, error);
        if (error) {
            result = path;
        }
    }

    std::string value = result.lexically_normal().string();
#ifdef _WIN32
    std::transform(value.begin(), value.end(), value.begin(),
        [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
#endif
    return value;
}

bool isWithin(const std::string& path, const fs::path& root) {
    const std::string normalizedRoot = normalizedPath(root);
    if (path == normalizedRoot) {
        return true;
    }

    std::string prefix = normalizedRoot;
    if (prefix.empty() || (prefix.back() != '/' && prefix.back() != '\\')) {
        prefix.push_back(fs::path::preferred_separator);
    }
    return path.compare(0, prefix.size(), prefix) == 0;
}

std::string declarationPath(const clang::Decl* declaration,
    const clang::SourceManager& sourceManager) {
    const clang::SourceLocation location =
        sourceManager.getFileLoc(declaration->getLocation());
    if (location.isInvalid()) {
        return {};
    }
    return normalizedPath(sourceManager.getFilename(location).str());
}

bool isAllowedPath(const std::string& path, const RunState& state) {
    if (path.empty()) {
        return false;
    }
    const bool selectedFile =
        state.selectedFiles.find(path) != state.selectedFiles.end();
    const bool inRoot = std::any_of(state.roots.begin(), state.roots.end(),
        [&](const fs::path& root) { return isWithin(path, root); });
    return state.roots.empty() ? selectedFile : inRoot;
}

const clang::NamedDecl* normalizedDeclaration(const clang::NamedDecl* declaration) {
    if (const auto* shadow = llvm::dyn_cast<clang::UsingShadowDecl>(declaration)) {
        declaration = shadow->getTargetDecl();
    }
    if (const auto* constructor =
            llvm::dyn_cast<clang::CXXConstructorDecl>(declaration)) {
        declaration = constructor->getParent();
    } else if (const auto* destructor =
                   llvm::dyn_cast<clang::CXXDestructorDecl>(declaration)) {
        declaration = destructor->getParent();
    }
    if (const auto* templateDecl = llvm::dyn_cast<clang::TemplateDecl>(declaration)) {
        declaration = templateDecl->getTemplatedDecl();
    }
    return declaration;
}

std::string declarationCategory(const clang::NamedDecl* declaration) {
    if (llvm::isa<clang::NamespaceDecl>(declaration)) {
        return "namespace";
    }
    if (llvm::isa<clang::TagDecl>(declaration)) {
        return "tag";
    }
    if (llvm::isa<clang::TypedefNameDecl>(declaration)) {
        return "typedef";
    }
    if (const auto* function = llvm::dyn_cast<clang::FunctionDecl>(declaration)) {
        return "function:" + function->getType().getCanonicalType().getAsString();
    }
    if (llvm::isa<clang::ValueDecl>(declaration)) {
        return "value";
    }
    return declaration->getDeclKindName();
}

bool makeSymbolKey(const clang::NamedDecl* declaration,
    const clang::SourceManager& sourceManager, SymbolKey& result) {
    declaration = normalizedDeclaration(declaration);
    if (declaration == nullptr || declaration->isImplicit() ||
        declaration->getIdentifier() == nullptr) {
        return false;
    }

    std::string identity = declaration->getQualifiedNameAsString();
    const auto* variable = llvm::dyn_cast<clang::VarDecl>(declaration);
    if ((variable != nullptr &&
            (llvm::isa<clang::ParmVarDecl>(variable) ||
                variable->getDeclContext()->isFunctionOrMethod())) ||
        declaration->getDeclContext()->isFunctionOrMethod()) {
        const clang::SourceLocation location =
            sourceManager.getFileLoc(declaration->getLocation());
        if (location.isInvalid()) {
            return false;
        }
        identity = declarationPath(declaration, sourceManager) + ":" +
            std::to_string(sourceManager.getFileOffset(location));
    }
    if (identity.empty()) {
        identity = declaration->getNameAsString();
    }

    result.value = declarationCategory(declaration) + "|" + identity;
    return true;
}

bool matchesOldName(const clang::NamedDecl* declaration,
    const std::string& oldName) {
    declaration = normalizedDeclaration(declaration);
    if (declaration == nullptr) {
        return false;
    }

    if (oldName.find("::") != std::string::npos) {
        return declaration->getQualifiedNameAsString() == oldName;
    }
    return declaration->getNameAsString() == oldName;
}

class CandidateCollector final : public MatchFinder::MatchCallback {
public:
    explicit CandidateCollector(RunState& state) : state_(state) {}

    void run(const MatchFinder::MatchResult& result) override {
        const auto* declaration =
            result.Nodes.getNodeAs<clang::NamedDecl>("candidate");
        if (declaration == nullptr || declaration->isImplicit() ||
            !matchesOldName(declaration, state_.oldName)) {
            return;
        }

        const std::string path =
            declarationPath(declaration, *result.SourceManager);
        if (!isAllowedPath(path, state_)) {
            return;
        }

        SymbolKey key;
        if (makeSymbolKey(declaration, *result.SourceManager, key)) {
            state_.candidates.insert(std::move(key));
        }
    }

private:
    RunState& state_;
};

class OccurrenceVisitor final
    : public clang::RecursiveASTVisitor<OccurrenceVisitor> {
public:
    OccurrenceVisitor(RunState& state, clang::ASTContext& context)
        : state_(state), sourceManager_(context.getSourceManager()),
          languageOptions_(context.getLangOpts()) {}

    bool VisitNamedDecl(clang::NamedDecl* declaration) {
        clang::SourceLocation location = declaration->getLocation();
        if (llvm::isa<clang::CXXDestructorDecl>(declaration) &&
            location.isValid() && !location.isMacroID()) {
            bool invalid = false;
            const char* spelling = sourceManager_.getCharacterData(location, &invalid);
            if (!invalid && spelling[0] == '~') {
                location = location.getLocWithOffset(1);
            }
        }
        addOccurrence(declaration, location);
        return true;
    }

    bool VisitDeclRefExpr(clang::DeclRefExpr* expression) {
        addOccurrence(expression->getDecl(), expression->getLocation());
        return true;
    }

    bool VisitMemberExpr(clang::MemberExpr* expression) {
        addOccurrence(expression->getMemberDecl(), expression->getMemberLoc());
        return true;
    }

    bool VisitUsingDirectiveDecl(clang::UsingDirectiveDecl* declaration) {
        addOccurrence(declaration->getNominatedNamespace(),
            declaration->getIdentLocation());
        return true;
    }

    bool TraverseTypeLoc(clang::TypeLoc typeLocation,
        bool traverseQualifier = true) {
        addTypeOccurrence(typeLocation);
        return clang::RecursiveASTVisitor<OccurrenceVisitor>::TraverseTypeLoc(
            typeLocation, traverseQualifier);
    }

    bool TraverseNestedNameSpecifierLoc(
        clang::NestedNameSpecifierLoc nameSpecifierLocation) {
        addNameSpecifierOccurrence(nameSpecifierLocation);
        return clang::RecursiveASTVisitor<OccurrenceVisitor>::
            TraverseNestedNameSpecifierLoc(nameSpecifierLocation);
    }

private:
    void addTypeOccurrence(clang::TypeLoc typeLocation) {
        if (typeLocation.isNull()) {
            return;
        }

        const clang::Type* type = typeLocation.getType().getTypePtrOrNull();
        if (type == nullptr) {
            return;
        }

        if (const auto tagLocation = typeLocation.getAs<clang::TagTypeLoc>()) {
            addOccurrence(tagLocation.getDecl(), tagLocation.getNameLoc());
        } else if (const auto typedefLocation =
                       typeLocation.getAs<clang::TypedefTypeLoc>()) {
            addOccurrence(typedefLocation.getTypePtr()->getDecl(),
                typedefLocation.getNameLoc());
        }

        if (const auto specialization =
                typeLocation.getAs<clang::TemplateSpecializationTypeLoc>()) {
            addOccurrence(
                specialization.getTypePtr()->getTemplateName().getAsTemplateDecl(),
                specialization.getTemplateNameLoc());
        } else if (const auto* parameter =
                       type->getAs<clang::TemplateTypeParmType>()) {
            if (const auto parameterLocation =
                    typeLocation.getAs<clang::TemplateTypeParmTypeLoc>()) {
                addOccurrence(parameter->getDecl(), parameterLocation.getNameLoc());
            }
        }
    }

    void addNameSpecifierOccurrence(
        clang::NestedNameSpecifierLoc nameSpecifierLocation) {
        if (!nameSpecifierLocation) {
            return;
        }

        const clang::NestedNameSpecifier nameSpecifier =
            nameSpecifierLocation.getNestedNameSpecifier();
        if (nameSpecifier.getKind() ==
            clang::NestedNameSpecifier::Kind::Namespace) {
            addOccurrence(
                nameSpecifier.getAsNamespaceAndPrefix().Namespace,
                nameSpecifierLocation.getLocalBeginLoc());
            return;
        }
        if (nameSpecifier.getKind() == clang::NestedNameSpecifier::Kind::Type) {
            const clang::Type* type = nameSpecifier.getAsType();
            if (const auto* tagType = type->getAs<clang::TagType>()) {
                addOccurrence(tagType->getDecl(),
                    nameSpecifierLocation.getLocalBeginLoc());
            } else if (const auto* typedefType =
                           type->getAs<clang::TypedefType>()) {
                addOccurrence(typedefType->getDecl(),
                    nameSpecifierLocation.getLocalBeginLoc());
            }
        }
    }

    void addOccurrence(const clang::NamedDecl* declaration,
        clang::SourceLocation location) {
        if (declaration == nullptr || location.isInvalid() || location.isMacroID()) {
            return;
        }

        SymbolKey key;
        if (!makeSymbolKey(declaration, sourceManager_, key) ||
            state_.candidates.find(key) == state_.candidates.end()) {
            return;
        }

        const clang::SourceLocation fileLocation = sourceManager_.getFileLoc(location);
        const std::string path =
            normalizedPath(sourceManager_.getFilename(fileLocation).str());
        if (!isAllowedPath(path, state_)) {
            return;
        }

        const unsigned length =
            clang::Lexer::MeasureTokenLength(fileLocation, sourceManager_,
                languageOptions_);
        if (length == 0) {
            state_.hadEditError = true;
            llvm::errs() << "Unable to determine identifier length at "
                         << path << ":" << sourceManager_.getSpellingLineNumber(fileLocation)
                         << "\n";
            return;
        }

        const unsigned offset = sourceManager_.getFileOffset(fileLocation);
        if (!state_.editLocations.emplace(path, offset).second) {
            return;
        }

        state_.edits[key].push_back({path, offset, length,
            sourceManager_.getSpellingLineNumber(fileLocation),
            sourceManager_.getSpellingColumnNumber(fileLocation),
            state_.newName});
    }

    RunState& state_;
    clang::SourceManager& sourceManager_;
    const clang::LangOptions& languageOptions_;
};

class RenamerConsumer final : public clang::ASTConsumer {
public:
    explicit RenamerConsumer(RunState& state) : state_(state) {}

    void HandleTranslationUnit(clang::ASTContext& context) override {
        CandidateCollector candidates(state_);
        MatchFinder finder;
        finder.addMatcher(
            clang::ast_matchers::namedDecl(
                clang::ast_matchers::unless(clang::ast_matchers::isImplicit()))
                .bind("candidate"),
            &candidates);
        finder.matchAST(context);

        OccurrenceVisitor visitor(state_, context);
        visitor.TraverseDecl(context.getTranslationUnitDecl());
    }

private:
    RunState& state_;
};

std::string symbolLeaf(const std::string& name) {
    const std::size_t separator = name.rfind("::");
    return separator == std::string::npos ? name : name.substr(separator + 2);
}

bool rewriteIncludeName(const std::string& fileName, const RunState& state,
    std::string& updatedFileName) {
    const std::string oldLeaf = symbolLeaf(state.oldName);
    updatedFileName = fileName;
    bool changed = false;

    std::size_t componentStart = 0;
    while (componentStart < updatedFileName.size()) {
        const std::size_t separator =
            updatedFileName.find_first_of("/\\", componentStart);
        const std::size_t componentEnd =
            separator == std::string::npos ? updatedFileName.size() : separator;
        const std::string component =
            updatedFileName.substr(componentStart, componentEnd - componentStart);
        std::size_t stemLength = component.size();
        if (separator == std::string::npos) {
            const std::size_t extension = component.find_last_of('.');
            if (extension != std::string::npos && extension != 0) {
                stemLength = extension;
            }
        }

        const bool matchingDirectory = separator != std::string::npos &&
            component == oldLeaf;
        const bool matchingHeader = separator == std::string::npos &&
            stemLength < component.size() &&
            component.substr(0, stemLength) == oldLeaf;
        const bool matchingPathWithoutExtension =
            separator == std::string::npos && component == oldLeaf;
        if (matchingDirectory || matchingHeader || matchingPathWithoutExtension) {
            updatedFileName.replace(
                componentStart, stemLength, state.newName);
            componentStart += state.newName.size();
            changed = true;
        } else {
            componentStart = componentEnd;
        }

        if (componentStart < updatedFileName.size() &&
            (updatedFileName[componentStart] == '/' ||
                updatedFileName[componentStart] == '\\')) {
            ++componentStart;
        }
    }

    return changed;
}

class IncludeUpdateCallbacks final : public clang::PPCallbacks {
public:
    IncludeUpdateCallbacks(RunState& state, clang::SourceManager& sourceManager,
        const clang::LangOptions& languageOptions)
        : state_(state), sourceManager_(sourceManager),
          languageOptions_(languageOptions) {}

    void InclusionDirective(clang::SourceLocation,
        const clang::Token&, llvm::StringRef fileName, bool isAngled,
        clang::CharSourceRange filenameRange,
        clang::OptionalFileEntryRef, llvm::StringRef, llvm::StringRef,
        const clang::Module*, bool,
        clang::SrcMgr::CharacteristicKind) override {
        std::string updatedFileName;
        if (!rewriteIncludeName(fileName.str(), state_, updatedFileName)) {
            return;
        }

        const clang::SourceLocation begin = filenameRange.getBegin();
        if (begin.isInvalid()) {
            reportFailure(begin, fileName, "source location is invalid");
            return;
        }
        const clang::SourceLocation directiveLocation =
            sourceManager_.getExpansionLoc(begin);
        const std::string path =
            normalizedPath(sourceManager_.getFilename(directiveLocation).str());
        if (!isAllowedPath(path, state_)) {
            return;
        }
        if (begin.isMacroID()) {
            reportFailure(begin, fileName, "include filename is macro-generated");
            return;
        }

        const clang::CharSourceRange fileRange = clang::Lexer::makeFileCharRange(
            filenameRange, sourceManager_, languageOptions_);
        if (fileRange.isInvalid()) {
            reportFailure(begin, fileName, "include filename range is not editable");
            return;
        }

        bool invalid = false;
        const llvm::StringRef originalText = clang::Lexer::getSourceText(
            fileRange, sourceManager_, languageOptions_, &invalid);
        if (invalid || originalText.empty()) {
            reportFailure(begin, fileName, "include filename text could not be read");
            return;
        }

        char delimiter = isAngled ? '<' : '"';
        char closingDelimiter = isAngled ? '>' : '"';
        if (originalText.size() >= 2 &&
            (originalText.front() == '"' || originalText.front() == '<') &&
            (originalText.back() == '"' || originalText.back() == '>')) {
            delimiter = originalText.front();
            closingDelimiter = originalText.back();
        }
        const std::string replacement =
            std::string(1, delimiter) + updatedFileName + closingDelimiter;

        const clang::SourceLocation fileBegin = fileRange.getBegin();
        const clang::SourceLocation fileEnd = fileRange.getEnd();
        const unsigned offset = sourceManager_.getFileOffset(fileBegin);
        const unsigned endOffset = sourceManager_.getFileOffset(fileEnd);
        if (endOffset <= offset) {
            reportFailure(begin, fileName, "include filename range has no editable text");
            return;
        }

        if (!state_.editLocations.emplace(path, offset).second) {
            return;
        }

        state_.includeEdits.push_back({path, offset, endOffset - offset,
            sourceManager_.getSpellingLineNumber(fileBegin),
            sourceManager_.getSpellingColumnNumber(fileBegin), replacement});
    }

private:
    void reportFailure(clang::SourceLocation location, llvm::StringRef fileName,
        const char* reason) {
        state_.hadEditError = true;
        if (location.isValid()) {
            location = sourceManager_.getExpansionLoc(location);
            llvm::errs() << "Unable to update include \"" << fileName << "\" at "
                         << sourceManager_.getFilename(location) << ":"
                         << sourceManager_.getSpellingLineNumber(location) << ": "
                         << reason << "\n";
        } else {
            llvm::errs() << "Unable to update include \"" << fileName
                         << "\": " << reason << "\n";
        }
    }

    RunState& state_;
    clang::SourceManager& sourceManager_;
    const clang::LangOptions& languageOptions_;
};

class RenamerAction final : public clang::ASTFrontendAction {
public:
    explicit RenamerAction(RunState& state) : state_(state) {}

    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
        clang::CompilerInstance& compiler, llvm::StringRef) override {
        compiler.getPreprocessor().addPPCallbacks(
            std::make_unique<IncludeUpdateCallbacks>(
                state_, compiler.getSourceManager(), compiler.getLangOpts()));
        return std::make_unique<RenamerConsumer>(state_);
    }

private:
    RunState& state_;
};

class RenamerActionFactory final : public clang::tooling::FrontendActionFactory {
public:
    explicit RenamerActionFactory(RunState& state) : state_(state) {}

    std::unique_ptr<clang::FrontendAction> create() override {
        return std::make_unique<RenamerAction>(state_);
    }

private:
    RunState& state_;
};

bool isSourceFile(const fs::path& path) {
    const std::string extension = path.extension().string();
    return extension == ".c" || extension == ".cc" || extension == ".cpp" ||
        extension == ".cxx" || extension == ".m" || extension == ".mm";
}

bool isValidIdentifier(const std::string& name) {
    if (name.empty()) {
        return false;
    }
    const auto isStart = [](unsigned char ch) {
        return ch == '_' || std::isalpha(ch) != 0;
    };
    const auto isPart = [](unsigned char ch) {
        return ch == '_' || std::isalnum(ch) != 0;
    };
    if (!isStart(static_cast<unsigned char>(name.front()))) {
        return false;
    }
    if (!std::all_of(name.begin() + 1, name.end(),
            [&](char ch) { return isPart(static_cast<unsigned char>(ch)); })) {
        return false;
    }
    clang::LangOptions languageOptions;
    languageOptions.CPlusPlus = true;
    languageOptions.CPlusPlus20 = true;
    clang::IdentifierTable identifiers(languageOptions);
    return !identifiers.get(name).isKeyword(languageOptions);
}

bool addRoot(RunState& state, const fs::path& path) {
    std::error_code error;
    if (!fs::is_directory(path, error) || error) {
        llvm::errs() << "Not a directory: " << path.string() << "\n";
        return false;
    }
    state.roots.push_back(path);
    return true;
}

bool prepareInputs(RunState& state, std::vector<std::string>& sources,
    std::unique_ptr<clang::tooling::CompilationDatabase>& database) {
    if (!RootPath.empty() && !addRoot(state, RootPath.getValue())) {
        return false;
    }

    for (const std::string& input : Inputs) {
        const fs::path path(input);
        std::error_code error;
        if (fs::is_directory(path, error) && !error) {
            if (!addRoot(state, path)) {
                return false;
            }
        } else {
            state.selectedFiles.insert(normalizedPath(path));
        }
    }

    if (state.roots.empty() && state.selectedFiles.empty()) {
        llvm::errs() << "Specify --root or at least one source file/folder.\n";
        return false;
    }

    std::string databaseError;
    database = clang::tooling::CompilationDatabase::autoDetectFromDirectory(
        BuildPath, databaseError);
    if (database == nullptr) {
        llvm::errs() << "Could not load compilation database from " << BuildPath
                     << ": " << databaseError << "\n";
        return false;
    }

    std::set<std::string> uniqueSources;
    for (const std::string& source : database->getAllFiles()) {
        const fs::path sourcePath(source);
        if (!isSourceFile(sourcePath)) {
            continue;
        }

        const std::string normalizedSource = normalizedPath(sourcePath);
        const bool selectedFile = state.selectedFiles.empty() ||
            state.selectedFiles.find(normalizedSource) != state.selectedFiles.end();
        const bool inRoot = state.roots.empty() ||
            std::any_of(state.roots.begin(), state.roots.end(),
                [&](const fs::path& root) { return isWithin(normalizedSource, root); });
        const bool inScope = selectedFile && inRoot;
        if (inScope && uniqueSources.insert(normalizedSource).second) {
            sources.push_back(source);
        }
    }

    if (sources.empty()) {
        llvm::errs() << "No source files from the compilation database matched "
                        "the requested scope.\n";
        return false;
    }
    return true;
}

bool applyEdits(const std::vector<Edit>& edits) {
    if (DryRun) {
        for (const Edit& edit : edits) {
            llvm::outs() << edit.path << ":" << edit.line << ":" << edit.column
                         << " -> " << edit.replacement << "\n";
        }
    }

    std::map<std::string, clang::tooling::Replacements> replacementsByFile;
    for (const Edit& edit : edits) {
        clang::tooling::Replacement replacement(
            edit.path, edit.offset, edit.length, edit.replacement);
        if (llvm::Error error =
                replacementsByFile[edit.path].add(replacement)) {
            llvm::errs() << "Conflicting rename edits in " << edit.path << ": "
                         << llvm::toString(std::move(error)) << "\n";
            return false;
        }
    }

    std::vector<std::pair<std::string, std::string>> updatedFiles;
    for (const auto& entry : replacementsByFile) {
        auto buffer = llvm::MemoryBuffer::getFile(entry.first);
        if (!buffer) {
            llvm::errs() << "Could not read " << entry.first << ": "
                         << buffer.getError().message() << "\n";
            return false;
        }
        auto updated = clang::tooling::applyAllReplacements(
            (*buffer)->getBuffer(), entry.second);
        if (!updated) {
            llvm::errs() << "Could not apply edits to " << entry.first << ": "
                         << llvm::toString(updated.takeError()) << "\n";
            return false;
        }
        updatedFiles.emplace_back(entry.first, std::move(*updated));
    }

    for (const auto& file : updatedFiles) {
        if (DryRun) {
            continue;
        }

        std::error_code error;
        llvm::raw_fd_ostream output(file.first, error, llvm::sys::fs::OF_None);
        if (error) {
            llvm::errs() << "Could not write " << file.first << ": "
                         << error.message() << "\n";
            return false;
        }
        output << file.second;
        output.close();
        if (output.has_error()) {
            llvm::errs() << "Failed while writing " << file.first << "\n";
            return false;
        }
        llvm::outs() << "Updated " << file.first << "\n";
    }
    return true;
}

}

int main(int argc, const char** argv) {
    llvm::cl::HideUnrelatedOptions(ToolCategory);
    llvm::cl::ParseCommandLineOptions(argc, argv,
        "Rename a C++ symbol using Clang's AST and compilation database, "
        "updating matching include paths.\n");

    std::string oldName = OldName;
    while (oldName.rfind("::", 0) == 0) {
        oldName.erase(0, 2);
    }
    if (oldName.empty()) {
        llvm::errs() << "--old must name a symbol.\n";
        return 2;
    }
    if (!isValidIdentifier(NewName)) {
        llvm::errs() << "--new must be a single unqualified C++ identifier.\n";
        return 2;
    }
    if (oldName == NewName) {
        llvm::errs() << "The old and new names are identical.\n";
        return 2;
    }

    RunState state;
    state.oldName = std::move(oldName);
    state.newName = NewName;
    std::unique_ptr<clang::tooling::CompilationDatabase> database;
    std::vector<std::string> sources;
    if (!prepareInputs(state, sources, database)) {
        return 2;
    }

    clang::tooling::ClangTool tool(*database, sources);
    RenamerActionFactory actionFactory(state);
    if (tool.run(&actionFactory) != 0) {
        llvm::errs() << "Clang failed to parse one or more translation units; "
                        "no files were changed.\n";
        return 1;
    }
    if (state.hadEditError) {
        llvm::errs() << "One or more symbol locations could not be edited; "
                        "no files were changed.\n";
        return 1;
    }
    if (state.candidates.empty()) {
        llvm::errs() << "No matching symbol was found in the selected scope: "
                     << state.oldName << "\n";
        return 1;
    }
    if (state.candidates.size() != 1) {
        llvm::errs() << "The name is ambiguous in the selected scope. Use a "
                        "qualified name or narrow the input files.\n";
        for (const SymbolKey& candidate : state.candidates) {
            llvm::errs() << "  " << candidate.value << "\n";
        }
        return 1;
    }

    const auto symbolEdits = state.edits.find(*state.candidates.begin());
    if (symbolEdits == state.edits.end() ||
        (symbolEdits->second.empty() && state.includeEdits.empty())) {
        llvm::errs() << "No editable occurrences were found for "
                     << state.oldName << "\n";
        return 1;
    }

    std::vector<Edit> edits = symbolEdits->second;
    edits.insert(edits.end(), state.includeEdits.begin(), state.includeEdits.end());
    if (!applyEdits(edits)) {
        return 1;
    }
    llvm::outs() << (DryRun ? "Would rename " : "Renamed ")
                 << state.oldName << " to " << state.newName << " in "
                 << edits.size() << " occurrence(s).\n";
    return 0;
}

#include "VariableNaming.h"

namespace clang {
namespace loop_migrate {

std::string VariableNamer::createIndexName() {
  // FIXME: Add in naming conventions to handle:
  //  - Uppercase/lowercase indices
  //  - How to handle conflicts
  //  - An interactive process for naming
  std::string IteratorName;
  std::string ContainerName;
  if (TheContainer)
    ContainerName = TheContainer->getName().str();

  size_t Len = ContainerName.length();
  if (Len > 1 && ContainerName[Len - 1] == 's')
    IteratorName = ContainerName.substr(0, Len - 1);
  else
    IteratorName = "elem";

  // FIXME: Maybe create a class so that this call doesn't need 6 parameters
  // every time?
  if (!declarationExists(IteratorName))
    return IteratorName;

  IteratorName = ContainerName + "_" + OldIndex->getName().str();
  if (!declarationExists(IteratorName))
    return IteratorName;

  IteratorName = ContainerName + "_elem";
  if (!declarationExists(IteratorName))
    return IteratorName;

  IteratorName += "_elem";
  if (!declarationExists(IteratorName))
    return IteratorName;

  IteratorName = "_elem_";

  // Someone defeated my naming scheme...
  while (declarationExists(IteratorName))
    IteratorName += "i";
  return IteratorName;
}

/// \brief Determines whether or not the the name Symbol exists in LoopContext,
/// any of its parent contexts, or any of its child statements.
///
/// We also check to see if the same identifier was generated by this loop
/// converter in a loop nested within SourceStmt.
bool VariableNamer::declarationExists(const std::string& Symbol) {
  IdentifierInfo& Identifier = Context->Idents.get(Symbol);
  DeclarationName Name =
      Context->DeclarationNames.getIdentifier(&Identifier);

  // First, let's check the parent context.
  // FIXME: lookup() always returns the pair (NULL, NULL) because its
  // StoredDeclsMap is not initialized (i.e. LookupPtr.getInt() is false inside
  // of DeclContext::lookup()). Why is this?
  // NOTE: We work around this by checking when a shadowed declaration is
  // referenced, rather than now.
  for (const DeclContext *CurrContext = LoopContext; CurrContext != NULL;
       CurrContext = CurrContext->getLookupParent()) {
    DeclContext::lookup_const_result Result = CurrContext->lookup(Name);
    if (Result.first != Result.second)
        return true;
  }

  // Determine if the symbol was generated in a parent context.
  for (const Stmt *S = SourceStmt; S != NULL; S = ReverseAST->lookup(S)) {
    StmtGeneratedVarNameMap::const_iterator I = GeneratedDecls->find(S);
    if (I != GeneratedDecls->end() && I->second == Symbol)
      return true;
  }

  // Finally, determine if the symbol was used in the loop or a child context.
  DeclFinderASTVisitor DeclFinder(Symbol, GeneratedDecls);
  return DeclFinder.findUsages(SourceStmt);
}

} // namespace loop_migrate
} // namespace clang

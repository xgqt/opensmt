#ifndef TERMMAPPER_H
#define TERMMAPPER_H
#include "Map.h"
#include "SolverTypes.h"
#include "Pterm.h"
#include "Logic.h"
class TermMapper {
  private:
    Logic&      logic;
  public:
    TermMapper(Logic& l) : logic(l) {}

    vec<PTRef>                                varToTerm;
    vec<SymRef>                               varToTheorySymbol;
    Map<PTRef,Var,PTRefHash,Equal<PTRef> >    termToVar;
    Map<PTRef,bool,PTRefHash,Equal<PTRef> >   theoryTerms;

    // Return a "purified" term by removing sequence of nots.  sgn is false if sequence length is even, and true if it odd
    void getTerm(PTRef tr, PTRef& tr_clean, bool& sgn) const;
    Var  getVar(PTRef)    const;
    Lit  getLit(PTRef)    const;
    bool hasLit(PTRef tr) const { return termToVar.contains(tr); }
#ifdef PEDANTIC_DEBUG
    Var  getVarDbg(int r) const { PTRef tr; tr = r; return termToVar[tr]; }
#endif
};

#endif

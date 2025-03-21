/*********************************************************************
Author: Antti Hyvarinen <antti.hyvarinen@gmail.com>

OpenSMT2 -- Copyright (C) 2012 - 2014 Antti Hyvarinen

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*********************************************************************/


#include "EnodeStore.h"
#include "Symbol.h"
#include "Logic.h"

EnodeStore::EnodeStore(Logic& l)
      : logic(l)
      , ea(1024*1024)
      , sig_tab(SignatureHash(ea), SignatureEqual(ea))
      , dist_idx(0)
{
    // For the uninterpreted predicates and propositional structures inside
    // uninterpreted functions, define function not, the terms true and false,
    // and an asserted disequality true != false

    PTRef t = logic.getTerm_true();
    PTRef f = logic.getTerm_false();
    constructTerm(t);
    constructTerm(f);
    ERef_True = termToERef[t];
    ERef_False = termToERef[f];
}

/**
 * Register term with this store.
 * This creates a new ENode representing the term, if the term has no been registered before.
 * @param term
 * @param ignoreChildren if true, creates a special version of ENode with no children
 * @return Reference to the newly created ENode
 */
ERef EnodeStore::addTerm(PTRef term, bool ignoreChildren) {
    if (termToERef.has(term))
        return termToERef[term];

    Pterm const & pterm = logic.getPterm(term);
    SymRef symref = pterm.symb();
    vec<ERef> args;
    auto argSpan = [&](){
        if (ignoreChildren) { return opensmt::span<ERef>(nullptr, 0); }
        args.capacity(pterm.nargs());
        for (PTRef arg : pterm) {
            assert(termToERef.has(arg));
            args.push(termToERef[arg]);
        }
        return opensmt::span(args.begin(), args.size_());
    }();
    ERef newEnode = ea.alloc(symref, argSpan, term);

    termToERef.insert(term, newEnode);
    assert(not ERefToTerm.has(newEnode));
    ERefToTerm.insert(newEnode, term);
    termEnodes.push(newEnode);
    return newEnode;
}

/**
 * Determine if a given term represented by the PTRef tr requires an enode term.
 * @param tr the PTRef
 * @return true tr needs an enode, false otherwise.
 * @note Could be implemented in Logic as well.
 */
bool EnodeStore::needsEnode(PTRef tr) const {
    if (logic.isConstant(tr)) {
        return true;
    } else if (logic.isVar(tr) and not logic.hasSortBool(tr)) {
        return true;
    } else if (logic.isUF(tr) or logic.yieldsSortUninterpreted(tr)) {
        return true;
    } else if (logic.hasArrays() and logic.isArraySort(logic.getSortRef(tr))) {
        return true;
    } else if (logic.isTheoryEquality(tr)) {
        return true;
    } else if (logic.appearsInUF(tr)) {
        return true;
    } else if (logic.isUP(tr)) {
        return true;
    } else if (logic.isDisequality(tr)) {
        return true;
    } else {
        return false;
    }
}

/**
 * Construct an Enode for a given PTRef, assuming that all the child PTRefs have
 * already been introduced an Enode.  In case of a Boolean return valued Enode,
 * add also an enode of the negation of the PTRef.  If the Boolean Enode is
 * non-atomic, no child Enodes will be constructed.
 * @param tr The PTRef for which the Enode(s) will be constructed
 * @return a vector of <PTRef,ERef> pairs consisting either of a single element
 * if the PTRef is non-boolean; two elements, the first of which corresponds to
 * the positive form and and the second the negated form of the PTRef tr; or is empty
 * if the PTRef has already been inserted.
 */
vec<PTRefERefPair> EnodeStore::constructTerm(PTRef tr) {

    assert(needsEnode(tr));

    if (termToERef.has(tr))
        return {};

    vec<PTRefERefPair> new_enodes;

    if (logic.isDisequality(tr)) {
        addDistClass(tr);
    }

    bool ignoreChildren = not needsRecursiveDefinition(tr);
    ERef er = addTerm(tr, ignoreChildren);
    new_enodes.push({tr, er});

    if (logic.hasSortBool(tr)) {
        // Add the negated term
        assert(logic.isBooleanOperator(tr) || logic.isBoolAtom(tr) || logic.isTrue(tr) || logic.isFalse(tr) || logic.isEquality(tr) || logic.isUP(tr) || logic.isDisequality(tr));
        assert(not logic.isNot(tr));
        PTRef tr_neg = logic.mkNot(tr);
        if (needsEnode(tr_neg)) {
            ERef er_neg = addTerm(tr_neg);
            new_enodes.push({tr_neg, er_neg});
        }
    }

    return new_enodes;
}

bool EnodeStore::needsRecursiveDefinition(PTRef tr) const {
    Pterm const & t = logic.getPterm(tr);
    return std::all_of(t.begin(), t.end(), [this](PTRef ch) { return needsEnode(ch); });
}

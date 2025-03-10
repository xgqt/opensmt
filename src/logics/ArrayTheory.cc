/*
 * Copyright (c) 2021, Martin Blicha <martin.blicha@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "ArrayTheory.h"
#include "DistinctRewriter.h"
#include "TreeOps.h"

vec<PTRef> collectStores(Logic const & logic, PTRef fla) {
    class CollectStoresConfig : public DefaultVisitorConfig {
        Logic const & logic;

    public:
        vec<PTRef> stores;

        CollectStoresConfig(Logic const & logic) : logic(logic) {}

        void visit(PTRef term) override {
            if (logic.isArrayStore(term)) {
                stores.push(term);
            }
        }
    };

    CollectStoresConfig config(logic);
    TermVisitor<CollectStoresConfig> visitor(logic, config);
    visitor.visit(fla);
    return std::move(config.stores);
}

PTRef instantiateReadOverStore(Logic & logic, PTRef fla) {
    vec<PTRef> stores = collectStores(logic, fla);
    vec<PTRef> instantiatedAxioms;
    for (PTRef store : stores) {
        PTRef index = logic.getPterm(store)[1];
        PTRef value = logic.getPterm(store)[2];
        assert(logic.isArrayStore(store));
        instantiatedAxioms.push(logic.mkEq(logic.mkSelect({store, index}), value));
    }
    instantiatedAxioms.push(fla);
    return logic.mkAnd(std::move(instantiatedAxioms));
}

bool ArrayTheory::simplify(vec<PFRef> const & formulas, PartitionManager &, int curr)
{
    // TODO: simplify select over store on the same index
    auto & currentFrame = pfstore[formulas[curr]];
    if (this->keepPartitions()) {
        std::logic_error("Not suppported yet");
    }
    PTRef rewritten = rewriteDistincts(getLogic(), getCollateFunction(formulas, curr));
    rewritten = instantiateReadOverStore(getLogic(), rewritten);
    currentFrame.root = rewritten;
    return true;
}

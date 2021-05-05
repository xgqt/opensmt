//
// Created by Martin Blicha on 14.06.20.
//

#ifndef OPENSMT_MODELBUILDER_H
#define OPENSMT_MODELBUILDER_H

#include "PTRef.h"
#include "Model.h"

#include <unordered_map>
#include <memory>

class Logic;

class ModelBuilder {
protected:
    std::unordered_map<PTRef, PTRef, PTRefHash> assignment;
    std::unordered_map<SymRef, TemplateFunction, SymRefHash> definitions;

    Logic& logic;

    int uniqueNum;

    const std::string formalArgDefaultPrefix;
public:

    ModelBuilder(Logic & logic) : logic(logic), uniqueNum(0), formalArgDefaultPrefix("x") {}

    void addVarValue(PTRef var, PTRef value) {
        auto res = assignment.insert(std::make_pair(var, value));
        assert(res.second); (void)res;
    }

    template<typename TIt>
    void addVarValues(TIt begin, TIt end) {
        assignment.insert(begin, end);
    }

    void addFunctionDefinition(SymRef sym, TemplateFunction templateFunction) {
        auto res = definitions.insert(std::make_pair(sym, templateFunction));
        assert(res.second); (void)res;
    }
    bool hasTheoryFunction(SymRef sr) const { return definitions.find(sr) != definitions.end();}
    bool hasTheoryFunction(PTRef tr) const { return hasTheoryFunction(logic.getSymRef(tr)); }

    void addToTheoryFunction(SymRef sr, vec<PTRef> vals, PTRef val);

    template<typename TIt>
    void addFunctionDefinitions(TIt begin, TIt end) {
        definitions.insert(begin, end);
    }

    std::unique_ptr<Model> build() const;
};


#endif //OPENSMT_MODELBUILDER_H

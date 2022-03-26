#pragma once
#include <functional>
#include <string>
#include <vector>
#include <unordered_set>
#include <utils.h>

class Query {
    public:
        std::vector<Variable> variables;
        std::unordered_set<TriplePattern> patterns;

        Query(std::vector<Variable> v, std::unordered_set<TriplePattern> p) :
            variables(v), patterns(p) {};
        static Query parse(std::string, std::function<Resource(std::string)>);
        std::vector<TriplePattern> plan();
};
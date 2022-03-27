#include <unordered_set>
#include <utils.h>

Term utils::apply_map(VariableMap map, Term term) {
    if (term.index() == 1) return term;
    else {
        Variable var = std::get<Variable>(term);
        if (map.count(var) == 0) return term;
        else return Term{map.at(var)};
    }
}

PatternType utils::get_pattern_type(TriplePattern pattern) {
    auto [a,b,c] = pattern;
    if (a.index() == 0) {
        if (b.index() == 0) return (c.index() == 0) ? XYZ : XYO;
        else return (c.index() == 0) ? XPZ : XPO;
    } else {
        if (b.index() == 0) return (c.index() == 0) ? SYZ : SYO;
        else return (c.index() == 0) ? SPZ : SPO;
    }
}

std::unordered_set<Variable> utils::get_variables(TriplePattern pattern) {
    std::unordered_set<Variable> output;
    auto [a,b,c] = pattern;
    if (a.index() == 0) output.insert(std::get<Variable>(a));
    if (b.index() == 0) output.insert(std::get<Variable>(b));
    if (c.index() == 0) output.insert(std::get<Variable>(c));
    return output;
}

template <class T>
std::unordered_set<T> utils::intersect(std::unordered_set<T> set1,
                                       std::unordered_set<T> set2) {
    std::unordered_set<T> output;
    for (T t1 : set1) {
        for (T t2 : set2) if (t1 == t2) output.insert(t1);
    }
    return output;
}

template std::unordered_set<Variable> utils::intersect(
    std::unordered_set<Variable>, std::unordered_set<Variable>);
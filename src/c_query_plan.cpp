#include <algorithm>
#include <exception>
#include <iostream>
#include <iterator>
#include <sstream>
#include <unordered_set>
#include <Query.h>
#include <utils.h>

std::vector<TriplePattern> Query::plan() {
    std::unordered_set<TriplePattern> unprocessed(patterns);
    std::vector<TriplePattern> processed;
    std::unordered_set<Variable> bound;

    while (!unprocessed.empty()) {
        int best_score = 100;
        TriplePattern best_pattern = INVALID_PATTERN;
        for (TriplePattern pattern : unprocessed) {
            int score = _get_score(pattern, bound);
            std::unordered_set<Variable> vars = utils::get_variables(pattern);
            bool condition = (score < best_score) &&
                (vars.empty() ||
                 !utils::intersect<Variable>(vars, bound).empty());
            if ((best_pattern == INVALID_PATTERN) || condition) {
                best_pattern = pattern;
                best_score = score;
            }
        }
        processed.push_back(best_pattern);
        std::unordered_set<Variable> vars = utils::get_variables(best_pattern);
        for (Variable var : vars) bound.insert(var);
        unprocessed.erase(best_pattern);
    }
    return processed;
}

int Query::_get_score(TriplePattern pattern,
                      std::unordered_set<Variable> bound) {
    auto is_bound = [=] (auto t) {
        return t.index() == 1 || bound.count(std::get<Variable>(t)); };
    auto [a,b,c] = pattern;
    if (is_bound(a)) {
        if (is_bound(b)) return (is_bound(c)) ? 1 : 4;
        else return (is_bound(c)) ? 2 : 6;
    } else {
        if (is_bound(b)) return (is_bound(c)) ? 3 : 7;
        else return (is_bound(c)) ? 5 : 8;
    }
}
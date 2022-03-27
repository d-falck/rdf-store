/**
 * @file c_query_plan.cpp
 * @author Candidate 1034792
 * @brief Implementation component (c)
 * 
 * The greedy join order optimisation query planner.
 * Partial implementation of the Query class, alongside `e_query_parse.cpp`.
 */
#include <algorithm>
#include <exception>
#include <iostream>
#include <iterator>
#include <sstream>
#include <unordered_set>
#include <Query.h>
#include <utils.h>

/**
 * @brief Plans execution ordern of the query according to a greedy heuristic
 * 
 * Applies the greedy join order optimisation algorithm suggested in Question
 * 1 of the paper based on the heuristic from the paper
 * 'Heuristics-based query optimisation for SPARQL'. This uses an approximate
 * ordering of triple pattern types to repeatedly pick a pattern with the
 * least expected join cost until all patterns have been processed.
 * 
 * @return std::vector<TriplePattern> 
 */
std::vector<TriplePattern> Query::plan() {
    // Maintain processed & unprocessed pattern sets and set of bound variables
    std::unordered_set<TriplePattern> unprocessed(patterns);
    std::vector<TriplePattern> processed;
    std::unordered_set<Variable> bound;

    // Repeatedly process a pattern until we have none left
    while (!unprocessed.empty()) {
        int best_score = 100;
        TriplePattern best_pattern = INVALID_PATTERN;

        // Pick the pattern with the lowest heuristic score
        for (TriplePattern pattern : unprocessed) {
            int score = _get_score(pattern, bound);
            std::unordered_set<Variable> vars = utils::get_variables(pattern);
            // Don't pick if it'll result in a cross product
            bool condition = (score < best_score) &&
                (vars.empty() ||
                 !utils::intersect<Variable>(vars, bound).empty());
            if ((best_pattern == INVALID_PATTERN) || condition) {
                best_pattern = pattern;
                best_score = score;
            }
        }

        // Process the chosen pattern and track the newly-bound variables
        processed.push_back(best_pattern);
        std::unordered_set<Variable> vars = utils::get_variables(best_pattern);
        for (Variable var : vars) bound.insert(var);
        unprocessed.erase(best_pattern);
    }
    return processed;
}

/**
 * @brief Calculates the heuristic score of a triple pattern
 * 
 * Assigns score based on position in the pattern type ordering suggested in
 * the paper Heuristics-based query optimisation for SPARQL'.
 * 
 * @param pattern Triple pattern to score
 * @param bound List of currently-bound variables; these will be treated as
 * resources if they occur in \p pattern
 * @return int Position in the query type ordering
 */
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
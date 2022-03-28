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
#include <list>
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
 * Note a slight modification has been made from the pseudocode given in the
 * exam paper; this implementation eliminates triple patterns that result in a
 * cross product *before* comparing scores. This is discussed further in the
 * accompanying report.
 * 
 * @return std::vector<TriplePattern> 
 */
std::vector<TriplePattern> Query::plan() {
    // Maintain processed & unprocessed pattern lists and set of bound variables
    std::vector<TriplePattern> unprocessed(patterns);
    std::vector<TriplePattern> processed;
    std::unordered_set<Variable> bound;
    
    // Repeatedly process a pattern until we have none left
    while (!unprocessed.empty()) {

        // Filter out patterns resulting in a cross product
        std::vector<TriplePattern> candidates;
        for (TriplePattern pattern : unprocessed) {
            std::unordered_set<Variable> vars = utils::get_variables(pattern);
            bool viable = vars.empty() || bound.empty() ||
                          !utils::intersect<Variable>(vars, bound).empty();
            if (viable) candidates.push_back(pattern);
        }
        
        // If none left we'll have to put up with a cross product
        if (candidates.empty()) candidates = unprocessed;

        // Now pick the pattern with the lowest heuristic score
        int best_score = 100;
        TriplePattern best_pattern = INVALID_PATTERN;
        for (TriplePattern pattern : candidates) {
            int score = _get_score(pattern, bound);
            std::unordered_set<Variable> vars = utils::get_variables(pattern);
            if (best_pattern == INVALID_PATTERN || score < best_score) {
                best_pattern = pattern;
                best_score = score;
            }
        }

        // Process the chosen pattern and track the newly-bound variables
        processed.push_back(best_pattern);
        std::unordered_set<Variable> vars = utils::get_variables(best_pattern);
        for (Variable var : vars) bound.insert(var);
        unprocessed.erase(
            std::remove(unprocessed.begin(), unprocessed.end(), best_pattern),
            unprocessed.end());
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
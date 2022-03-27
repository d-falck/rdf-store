/**
 * @file Query.h
 * @author Candidate 1034792
 * @brief Declaration of the Query class
 */
#pragma once
#include <functional>
#include <string>
#include <vector>
#include <unordered_set>
#include <utils.h>

/**
 * @brief A single parsed SPARQL query
 * 
 * The Query class represents a single parsed BGP SPARQL query, also providing
 * query parsing and join order planning functionality.
 * 
 * Member function documentation provided in implementation files
 * `c_query_plan.cpp` and `e_query_parse.cpp`.
 */
class Query {
    public:
        // Variables in this query (in order)
        std::vector<Variable> variables;
        // Patterns in this query (with no order)
        std::unordered_set<TriplePattern> patterns;

        Query(std::vector<Variable> v, std::unordered_set<TriplePattern> p) :
            variables(v), patterns(p) {};
        static Query parse(std::string, std::function<Resource(std::string)>);
        std::vector<TriplePattern> plan();

    private:
        static int _get_score(TriplePattern, std::unordered_set<Variable>);
        static Variable _parse_variable(std::string);
        static Term _parse_term(std::string,
                                std::function<Resource(std::string)>);
};
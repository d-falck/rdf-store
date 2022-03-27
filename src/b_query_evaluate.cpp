/**
 * @file b_query_evaluate.cpp
 * @author Candidate 1034792
 * @brief Implementation component (b)
 * 
 * The engine for evaluating BGP SPARQL queries.
 * Partial implementation of the System class, alongside `d_turtle_parse.cpp`.
 */
#include <chrono>
#include <exception>
#include <iostream>
#include <sstream>
#include <tuple>
#include <System.h>
#include <Query.h>
#include <utils.h>

/**
 * @brief Evaluates a BGP SPARQL query string over currently stored triples
 * 
 * Prints number of results and time taken to stdout, optionally also printing
 * the query results themselves. (This is optional to facilitate timing.)
 * Implements the query evaluation algorithm suggested in Question 1
 * of the paper.
 * 
 * @param query_string BGP SPARQL query string to be evalauted
 * @param print Whether to print individual results (as opposed to just
 *      the result count and time taken)
 */
void System::evaluate_query(std::string query_string, bool print) {
    auto start = std::chrono::high_resolution_clock::now();

    // Parse query and run join order optimizer
    Query query = Query::parse(query_string, [=] (std::string name) {
        return _encode_resource(name); });
    std::vector<TriplePattern> patterns = query.plan();
    std::vector<Variable> variables = query.variables;
    VariableMap map;

    // Initiate recursive join
    if (print) {
        std::cout << "----------" << std::endl;
        for (Variable var : variables)
            std::cout << "?" << var << "\t";
        std::cout << std::endl;
    }
    _result_counter = 0;
    _nested_index_loop_join(map, 0, print, patterns, variables);
    if (print) std::cout << "----------" << std::endl;

    // Summarize output
    auto end = std::chrono::high_resolution_clock::now();
    int elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>
        (end-start).count();
    std::cout << _result_counter << " results returned in "
              << elapsed_ms << " ms." << std::endl;
}

/**
 * @brief Recursive helper function for System::evaluate_query
 * 
 * Performs a recursive nested index loop join on patterns \p i onwards
 * with currently assigned variable bindings \p map. Implements the algorithm
 * described in Question 1 of the paper.
 * 
 * @param map Already-determined variable mappings to join with
 * @param i Index of first pattern to join with current bindings
 * @param print Whether to print the results (if no patterns left to join)
 * @param patterns Full list of patterns to evaluate, including ones
 *      already processed
 * @param variables List of variables we wish to map to resources
 */
void System::_nested_index_loop_join(VariableMap& map, int i, bool print,
                                     std::vector<TriplePattern> patterns,
                                     std::vector<Variable> variables) {
    if (i == patterns.size()) {
        _result_counter++;
        if (print) _print_mapped_values(map, variables);
    } else {
        auto [a,b,c] = patterns[i];
        // Get iterator over variable mappings matching this pattern
        std::function<std::optional<VariableMap>()> generate = _index.evaluate(
            utils::apply_map(map, a), utils::apply_map(map, b),
            utils::apply_map(map, c));
        // Make recursive call for each map in the iterator
        std::optional<VariableMap> rho;
        while ((rho = generate()).has_value()) {
            for (auto [var, res] : *rho) map[var] = res; // Add to map
            _nested_index_loop_join(map, i+1, print, patterns, variables);
            for (auto [var, res] : *rho) map.erase(var); // Remove from map
        }
    }
}

/**
 * @brief Helper function to apply a mapping to variables and print the results
 * 
 * Requires access to the underlying System object so that Resource integer
 * representations can be decoded into URI strings.
 * 
 * @param map 
 * @param variables 
 */
void System::_print_mapped_values(VariableMap map,
                                  std::vector<Variable> variables) {
    for (Variable var : variables) {
        if (map.count(var) == 0)
            throw std::invalid_argument("Map doesn't contain all variables");
        std::cout << _decode_resource(map[var]) << "\t";
    }
    std::cout << std::endl;
}
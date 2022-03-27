#include <chrono>
#include <exception>
#include <iostream>
#include <sstream>
#include <tuple>
#include <System.h>
#include <Query.h>
#include <utils.h>

void System::evaluate_query(std::string query_string, bool print) {
    auto start = std::chrono::high_resolution_clock::now();

    Query query = Query::parse(query_string, [=] (std::string name) {
        return _encode_resource(name); });
    std::vector<TriplePattern> patterns = query.plan();
    std::vector<Variable> variables = query.variables;
    VariableMap map;

    if (print) {
        std::cout << "----------" << std::endl;
        for (Variable var : variables)
            std::cout << "?" << var << "\t";
        std::cout << std::endl;
    }
    _result_counter = 0;
    _nested_index_loop_join(map, 0, print, patterns, variables);
    if (print) std::cout << "----------" << std::endl;

    auto end = std::chrono::high_resolution_clock::now();
    int elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>
        (end-start).count();
    std::cout << _result_counter << " results returned in "
              << elapsed_ms << " ms." << std::endl;
}

void System::_nested_index_loop_join(VariableMap& map, int i, bool print,
                                     std::vector<TriplePattern> patterns,
                                     std::vector<Variable> variables) {
    if (i == patterns.size()) {
        _result_counter++;
        if (print) _print_mapped_values(map, variables);
    } else {
        TriplePattern pattern = patterns[i];
        Term a = std::get<0>(pattern);
        Term b = std::get<1>(pattern);
        Term c = std::get<2>(pattern);
        Term sigma_a = utils::apply_map(map, a);
        Term sigma_b = utils::apply_map(map, b);
        Term sigma_c = utils::apply_map(map, c);
        std::function<std::optional<VariableMap>()> generate = _index.evaluate(
            sigma_a, sigma_b, sigma_c);
        std::optional<VariableMap> rho;
        while ((rho = generate()).has_value()) {
            for (auto [var, res] : *rho) map[var] = res;
            _nested_index_loop_join(map, i+1, print, patterns, variables);
            for (auto [var, res] : *rho) map.erase(var);
        }
    }
}

void System::_print_mapped_values(VariableMap map,
                                  std::vector<Variable> variables) {
    for (Variable var : variables) {
        if (map.count(var) == 0)
            throw std::invalid_argument("Map doesn't contain all variables");
        std::cout << _decode_resource(map[var]) << "\t";
    }
    std::cout << std::endl;
}
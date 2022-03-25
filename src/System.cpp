#include <chrono>
#include <exception>
#include <iostream>

#include <System.h>

void System::select_query_string(std::string query_string) {
    Query query = Query::parse(query_string,
        [=] (std::string name) {return _encode_resource(name);});
    _evaluate_query(query, true);
}

void System::select_query_string(std::string query_string) {
    Query query = Query::parse(query_string,
        [=] (std::string name) {return _encode_resource(name);});
    _evaluate_query(query, false);
}

void System::_evaluate_query(Query query, bool print) {
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<TriplePattern> patterns = query.optimize_join_order();
    std::vector<Variable> variables = query.get_variables();
    VariableMap map;

    if (print) {
        std::cout << "----------" << std::endl;
        for (Variable var : variables)
            std::cout << "?" << (char) var << " " << std::endl;
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
        std::vector<TriplePattern> patterns, std::vector<Variable> variables) {
    if (i == variables.size()) {
        _result_counter++;
        if (print) _print_mapped_values(map, variables);
    } else {
        auto [a, b, c] = patterns[i];
        std::function<std::optional<VariableMap>()> generate = _index.evaluate(
            _apply_map(map, a), _apply_map(map, b), _apply_map(map, c)
        );
        std::optional<VariableMap> rho;
        while ((rho = generate()).has_value()) {
            for (auto [var, res] : rho.value()) map[var] = res;
            _nested_index_loop_join(map, i+1, print, patterns, variables);
            for (auto [var, res] : rho.value()) map.erase(var);
        }
    }
}

void System::_print_mapped_values(VariableMap map,
                                  std::vector<Variable> variables) {
    for (Variable var : variables) {
        if (map.count(var) == 0)
            throw std::invalid_argument("Map doesn't contain all variables");
        std::cout << _decode_resource(map[var]) << " ";
    }
    std::cout << std::endl;
}

Term _apply_map(VariableMap map, Term term) {
    if (term.index() == 1) return term;
    else {
        Variable var = std::get<Variable>(term);
        return Term{map[var]};
    }
}

Resource System::_encode_resource(std::string name) {
    int n = name.length();
    bool valid = (name[0] == '<' && name[n-1] == '>') ||
                 (name[0] == '"' && name[n-1] == '"');
    if (!valid) throw std::invalid_argument(
        "Resources must be enclosed in quotes or angle brackets");
    if (_resource_ids.count(name) == 0) {
        _stored_resources.push_back(name);
        _resource_ids[name] = _stored_resources.size()-1;
    }
    return _resource_ids[name];
}

std::string System::_decode_resource(Resource id) {
    if (id >= _stored_resources.size())
        throw std::invalid_argument("Resource ID does not exist");
    return _stored_resources[id];
}
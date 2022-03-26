#include <chrono>
#include <exception>
#include <iostream>
#include <sstream>
#include <tuple>
#include <System.h>

void System::select_query_string(std::string query_string) {
    Query query = Query::parse(query_string,
        [=] (std::string name) {return _encode_resource(name);});
    _evaluate_query(query, true);
}

void System::count_query_string(std::string query_string) {
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
            std::cout << "?" << (char) var << "\t";
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

Term _apply_map(VariableMap map, Term term) {
    if (term.index() == 1) return term;
    else {
        Variable var = std::get<Variable>(term);
        if (map.count(var) == 0) return term;
        else return Term{map.at(var)};
    }
}

void System::_nested_index_loop_join(VariableMap& map, int i, bool print,
        std::vector<TriplePattern> patterns, std::vector<Variable> variables) {
    if (i == variables.size()) {
        _result_counter++;
        if (print) _print_mapped_values(map, variables);
    } else {
        TriplePattern pattern = patterns[i];
        Term a = std::get<0>(pattern);
        Term b = std::get<1>(pattern);
        Term c = std::get<2>(pattern);
        std::function<std::optional<VariableMap>()> generate = _index.evaluate(
            _apply_map(map, a), _apply_map(map, b), _apply_map(map, c)
        );
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

void System::load_triples(std::string str) {
    auto start = std::chrono::high_resolution_clock::now();

    // Split by whitespace
    std::stringstream stream(str);
    std::vector<std::string> words;
    for (std::string word; stream>>word;) words.push_back(word);

    // Check it's the right length
    if (words.size() % 4 != 0)
        throw std::invalid_argument("Invalid N-Triples syntax");

    // Parse each resource
    for (int i=0; i < words.size(); i += 4) {
        if (words[i+3] != ".")
            throw std::invalid_argument("Triples must be separated by periods");
        Resource s = _encode_resource(words[i]);
        Resource p = _encode_resource(words[i+1]);
        Resource o = _encode_resource(words[i+2]);
        _index.add(s, p, o);
    }

    auto end = std::chrono::high_resolution_clock::now();
    int elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>
        (end-start).count();
    std::cout << words.size()/4 << " triples loaded in "
              << elapsed_ms <<" ms." << std::endl;
}
#include <System.h>


std::function<std::optional<VariableMap>()> System::evaluate_query(Query query) {
    std::vector<TriplePattern> patterns = query.optimize_join_order();
    std::vector<Variable> variables = query.get_variables();
    int m = patterns.size();
    int n = variables.size();
    VariableMap map;

    return _nested_index_loop_join(map, 0, n, m, patterns, variables);
}

std::function<std::optional<VariableMap>()> System::_nested_index_loop_join(
        VariableMap& map, int i, int n, int m,
        std::vector<TriplePattern> patterns, std::vector<Variable> variables) {
    if (i == m+1) {
        int done = false;
        return [=] () mutable {
            if (!done) {
                done = true;
                return std::make_optional<VariableMap>(_project(map, variables));
            } else return std::optional<VariableMap>();
        };
    } else {
        auto [a, b, c] = patterns[i];
        std::function<std::optional<VariableMap>()> generate = _index.evaluate(
            _apply_map(map, a), _apply_map(map, b), _apply_map(map, c)
        );
        std::optional<VariableMap> rho(VariableMap{});
        std::function<std::optional<VariableMap>()> func;
        std::optional<VariableMap> out;
        return [=] () mutable {
            while (!out.has_value()) {
                for (auto [var, res] : rho.value()) map.erase(var);
                rho = generate();
                if (!rho.has_value()) return std::optional<VariableMap>();
                for (auto [var, res] : rho.value()) map[var] = res;
                func = _nested_index_loop_join(map, i+1, n, m, patterns, variables);
                out = func();
            }
            VariableMap out_map = out.value();
            out = func();
            return std::make_optional<VariableMap>(out_map);
        };
    }
}

VariableMap _project(VariableMap map, std::vector<Variable> variables) {
    VariableMap output;
    for (Variable var : variables) {
        if (map.count(var) == 0) throw std::invalid_argument("Invalid projection");
        output[var] = map[var];
    }
    return output;
}

Term _apply_map(VariableMap map, Term term) {
    if (term.index() == 1) return term;
    else {
        Variable var = std::get<Variable>(term);
        return Term{map[var]};
    }
}
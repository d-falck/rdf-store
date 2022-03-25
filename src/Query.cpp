#include <algorithm>
#include <exception>
#include <iostream>
#include <iterator>
#include <sstream>
#include <Query.h>

Query::Query(std::vector<Variable> vars,
             std::unordered_set<TriplePattern> pats) {
    _variables = vars;
    _patterns = pats;
}

Variable _parse_variable(std::string str) {
    if (str[0] != '?') throw std::invalid_argument(
        "Variable doesn't begin with ?");
    if (str.length() != 2) throw std::invalid_argument(
        "Variable names must be single characters");
    return (Variable) str[1];
}

Term _parse_term(std::string str,
                 std::function<Resource(std::string)> resource_encoder) {
    if (str[0] == '?') return Term{_parse_variable(str)};
    else return Term{resource_encoder(str)};
}

Query Query::parse(std::string query_string,
                   std::function<Resource(std::string)> resource_encoder) {
    // TODO: first ensure whitespace is correctly present in query string

    // Parse into vector of words
    std::stringstream stream(query_string);
    std::vector<std::string> words;
    for (std::string word; stream>>word;) words.push_back(word);

    // Split up vector of words into sections
    auto where = std::find(words.begin(), words.end(), "WHERE");
    if (where == words.end()) throw std::invalid_argument(
        "Query needs a WHERE clause");
    int where_loc = std::distance(words.begin(), where);
    if (words[where_loc+1] != "{") throw std::invalid_argument(
        "No opening brace in query");
    int end_loc = std::distance(words.begin(), words.end());
    if (words[end_loc-1] == ";") throw std::invalid_argument(
        "No semicolon allowed after query");
    if (words[end_loc-1] != "}") throw std::invalid_argument(
        "Query must end in closing brace");
    if ((end_loc-where_loc-3) % 4 != 0) throw std::invalid_argument(
        "Invalid sequence of patterns");

    // Get variables
    std::vector<Variable> vars;
    for (int i=0; i<where_loc; i++) vars.push_back(_parse_variable(words[i]));

    // Get triple patterns    
    std::unordered_set<TriplePattern> pats;
    for (int i=where_loc+2; i<end_loc-1; i+=4) {
        if (words[i+3] != ".") throw std::invalid_argument(
            "Pattern doesn't end in .");
        Term a = _parse_term(words[i], resource_encoder);
        Term b = _parse_term(words[i+1], resource_encoder);
        Term c = _parse_term(words[i+2], resource_encoder);
        pats.insert(std::make_tuple(a, b, c));
    }

    return Query(vars, pats);
}

std::vector<Variable> Query::get_variables() {return _variables;}
std::unordered_set<TriplePattern> Query::get_patterns() {return _patterns;}

int _get_score(TriplePattern pattern, std::unordered_set<Variable> bound) {
    auto [a,b,c] = pattern;
    bool a_is_bound = (a.index() == 1 || bound.count(std::get<Variable>(a)) == 1);
    bool b_is_bound = (b.index() == 1 || bound.count(std::get<Variable>(b)) == 1);
    bool c_is_bound = (c.index() == 1 || bound.count(std::get<Variable>(c)) == 1);
    if (a_is_bound) {
        if (b_is_bound) return (c_is_bound) ? 1 : 4;
        else return (c_is_bound) ? 2 : 6;
    } else {
        if (b_is_bound) return (c_is_bound) ? 3 : 7;
        else return (c_is_bound) ? 5 : 8;
    }
}

std::unordered_set<Variable> _get_variables(TriplePattern pattern) {
    std::unordered_set<Variable> output;
    auto [a,b,c] = pattern;
    if (a.index() == 0) output.insert(std::get<Variable>(a));
    if (b.index() == 0) output.insert(std::get<Variable>(b));
    if (c.index() == 0) output.insert(std::get<Variable>(c));
    return output;
}

template <class T>
std::unordered_set<T> _intersect(std::unordered_set<T> set1,
                                 std::unordered_set<T> set2) {
    std::unordered_set<T> output;
    for (T t1 : set1) {
        for (T t2 : set2) if (t1 == t2) output.insert(t1);
    }
    return output;
}

std::vector<TriplePattern> Query::optimize_join_order() {
    std::unordered_set<TriplePattern> unprocessed(_patterns);
    std::vector<TriplePattern> processed;
    std::unordered_set<Variable> bound;

    while (!unprocessed.empty()) {
        int best_score = 100;
        TriplePattern best_pattern = INVALID_PATTERN;
        for (TriplePattern pattern : unprocessed) {
            int score = _get_score(pattern, bound);
            std::unordered_set<Variable> vars = _get_variables(pattern);
            bool condition = (score < best_score)
                && (vars.empty() || !_intersect<Variable>(vars, bound).empty());
            if ((best_pattern == INVALID_PATTERN) || condition) {
                best_pattern = pattern;
                best_score = score;
            }
        }
        processed.push_back(best_pattern);
        for (Variable var : _get_variables(best_pattern)) bound.insert(var);
        unprocessed.erase(best_pattern);
    }
    return processed;
}
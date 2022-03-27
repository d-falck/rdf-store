/**
 * @file e_query_parse.cpp
 * @author Candidate 1034792
 * @brief Implementation component (e)
 * 
 * The parser for SPARQL queries.
 * Partial implementation of the Query class, alongside `c_query_plan.cpp`.
 */
#include <exception>
#include <iostream>
#include <iterator>
#include <sstream>
#include <Query.h>
#include <utils.h>

/**
 * @brief Constructs a Query object from a SPARQL query string
 * 
 * As this method doesn't have access to the System object from which it is
 * likely to be called, it requires a \p resource_encoder function to be passed
 * which encodes resource URIs into integer IDs.
 * 
 * @param query_string BGP SPARQL query string to be parsed
 * @param resource_encoder Function which encodes resource URIs into integer IDs
 * @return Query Object representing this query
 */
Query Query::parse(std::string query_string,
                   std::function<Resource(std::string)> resource_encoder) {
    // Ensure braces are surrounded by whitespace (for stream later)
    size_t pos;
    if ((pos=query_string.find('{')) == query_string.npos)
        throw std::invalid_argument("No opening brace in query");
    query_string.insert(pos+1, " ");
    if ((pos=query_string.find('}')) == query_string.npos)
        throw std::invalid_argument("No closing brace in query");
    query_string.insert(pos, " ");

    // Parse into vector of words
    std::stringstream stream(query_string);
    std::vector<std::string> words;
    for (std::string word; stream>>word;) words.push_back(word);

    // Split up vector of words into sections
    auto where = std::find(words.begin(), words.end(), "WHERE");
    if (where == words.end()) 
        throw std::invalid_argument("Query needs a WHERE clause");
    int where_loc = std::distance(words.begin(), where);
    if (words[where_loc+1] != "{")
        throw std::invalid_argument("Misplaced opening brace");
    int end_loc = std::distance(words.begin(), words.end());
    if (words[end_loc-1] == ";")
        throw std::invalid_argument("No semicolon allowed after query");
    if (words[end_loc-1] != "}") 
        throw std::invalid_argument("Misplaced closing brace");
    if ((end_loc-where_loc-3) % 4 != 0) 
        throw std::invalid_argument("Invalid sequence of patterns");
    if (end_loc-where_loc-3 == 0)
        throw std::invalid_argument("No patterns given!");

    // Get variables
    std::vector<Variable> vars;
    for (int i=0; i<where_loc; i++) vars.push_back(_parse_variable(words[i]));

    // Get triple patterns    
    std::unordered_set<TriplePattern> pats;
    for (int i=where_loc+2; i<end_loc-1; i+=4) {
        if (words[i+3] != ".")
            throw std::invalid_argument("Pattern doesn't end in .");
        Term a = _parse_term(words[i], resource_encoder);
        Term b = _parse_term(words[i+1], resource_encoder);
        Term c = _parse_term(words[i+2], resource_encoder);
        pats.insert(std::make_tuple(a, b, c));
    }

    return Query(vars, pats);
}

/**
 * @brief Helper function to parse a variable from a string
 * 
 * @param str String representing a variable, must begin with `?`
 * @return Variable Object representing this variable
 */
Variable Query::_parse_variable(std::string str) {
    if (str[0] != '?')
        throw std::invalid_argument("Variable doesn't begin with ?");
    return (Variable) str.substr(0, str.npos);
}

/**
 * @brief Helper function to parse a term from a string
 * 
 * Requires access to a \p resource_encoder callable
 * 
 * @param str String representing a resource or variable. Either begins in `?`
 *      or is wrapped in `""` or `<>`
 * @param resource_encoder Function which encodes resource URIs into integer IDs
 * @return Term Object representing this variable or resource
 */
Term Query::_parse_term(std::string str,
                        std::function<Resource(std::string)> resource_encoder) {
    if (str[0] == '?') return Term{_parse_variable(str)};
    else return Term{resource_encoder(str)};
}
/**
 * @file System.h
 * @author Candidate 1034792
 * @brief Declaration of the System class
 */
#pragma once
#include <functional>
#include <optional>
#include <RDFIndex.h>
#include <utils.h>

/**
 * @brief Overall RDF store system
 * 
 * The System class represents the RDF store as a whole, providing
 * an interface for the CLI and managing all relevant states, as well as
 * providing logic for query evaluation, triple loading
 * and resource encoding/decoding.
 * 
 * Member function documentation provided in implementation files
 * `b_query_evaluate.cpp` and `d_turtle_parse.cpp`.
 */
class System {
    public:
        void evaluate_query(std::string, bool);
        void load_triples(std::string);

    private:
        // RDF triple storage index
        RDFIndex _index; 
        // Counter for use when evaluating queries
        int _result_counter; 
        // Int-to-string and string-to-int resource maps
        std::vector<std::string> _stored_resources;
        std::unordered_map<std::string, Resource> _resource_ids;

        void _nested_index_loop_join(VariableMap&, int, bool,
                                     std::vector<TriplePattern>,
                                     std::vector<Variable>);
        void _print_mapped_values(VariableMap, std::vector<Variable>);
        Resource _encode_resource(std::string);
        std::string _decode_resource(Resource);
        std::string _term_to_string(Term);
};
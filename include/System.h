#pragma once

#include <functional>
#include <optional>

#include <types.h>
#include <RDFIndex.h>
#include <Query.h>


class System {
    public:
        void select_query_string(std::string);
        void count_query_string(std::string);
        void load_triples(std::string);

    private:
        RDFIndex _index;
        int _result_counter;
        std::vector<std::string> _stored_resources;
        std::unordered_map<std::string, Resource> _resource_ids;

        void _evaluate_query(Query, bool);
        void _nested_index_loop_join(VariableMap&, int, bool,
            std::vector<TriplePattern>, std::vector<Variable>);
        void _print_mapped_values(VariableMap, std::vector<Variable>);
        Resource _encode_resource(std::string);
        std::string _decode_resource(Resource);
};
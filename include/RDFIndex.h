/**
 * @file RDFIndex.h
 * @author Candidate 1034792
 * @brief Declaration of the RDFIndex class
 */
#pragma once
#include <functional>
#include <optional>
#include <tuple>
#include <unordered_map>
#include <vector>
#include <utils.h>

/**
 * @brief RDF indexing data structure
 * 
 * The RDFIndex class represents the indexing data structure for
 * storage of and access to RDF triples.
 * 
 * Member function documentation provided in implementation file `a_index.cpp`.
 */
class RDFIndex {
    public:
        ~RDFIndex();
        void add(Resource, Resource, Resource);
        std::function<std::optional<VariableMap>()> evaluate(Term, Term, Term);

    private:
        // Represents a single row in the triple table
        struct _TableRow { 
            Resource s, p, o;
            _TableRow *next_SP, *next_OP, *next_P;
        };

        // Triple table
        std::vector<_TableRow*> _table;
        // Index structures as specified in the paper this is based on
        std::unordered_map<Resource, _TableRow*> _index_S, _index_O, _index_P;
        std::unordered_map<ResourcePair, _TableRow*> _index_SP, _index_OP;
        std::unordered_map<ResourceTriple, _TableRow*> _index_SPO;
        // Length counters for SP- and OP-lists
        std::unordered_map<_TableRow*, size_t> _len_S, _len_O;

};
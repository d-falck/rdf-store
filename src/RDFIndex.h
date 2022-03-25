#include <functional>
#include <optional>
#include <tuple>
#include <unordered_map>
#include <vector>

#include <types.h>


class RDFIndex {
    public:
        ~RDFIndex();
        void add(Resource, Resource, Resource);
        std::function<std::optional<VariableMap>()> evaluate(Term, Term, Term);

    private:
        struct _TableRow {
            Resource s, p, o;
            _TableRow *next_SP, *next_OP, *next_P;
        };

        std::vector<_TableRow*> _table;
        std::unordered_map<Resource, _TableRow*> _index_S, _index_O, _index_P;
        std::unordered_map<ResourcePair, _TableRow*> _index_SP, _index_OP;
        std::unordered_map<ResourceTriple, _TableRow*> _index_SPO;

};
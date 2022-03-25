#include <functional>
#include <optional>

#include <types.h>
#include <RDFIndex.h>
#include <Query.h>


class System {
    public:
        std::function<std::optional<VariableMap>()> evaluate_query(Query);
        void print_select_query(std::string);
        void print_count_query(std::string);

    private:
        RDFIndex _index;
        std::function<std::optional<VariableMap>()> _nested_index_loop_join(
            VariableMap&, int, int, int,
            std::vector<TriplePattern>, std::vector<Variable>);
        
};
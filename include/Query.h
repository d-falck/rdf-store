#include <functional>
#include <string>
#include <vector>
#include <unordered_set>

#include <types.h>


class Query {
    public:
        Query(std::vector<Variable>, std::unordered_set<TriplePattern>);
        static Query parse(std::string, std::function<Resource(std::string)>);

        std::vector<Variable> get_variables();
        std::unordered_set<TriplePattern> get_patterns();
        std::vector<TriplePattern> optimize_join_order();

    private:
        std::vector<Variable> _variables;
        std::unordered_set<TriplePattern> _patterns;
};
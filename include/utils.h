#pragma once
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <variant>

// Types
using Resource = int;
using Variable = std::string;
using Term = std::variant<Variable, Resource>;
using ResourcePair = std::tuple<Resource, Resource>;
using ResourceTriple = std::tuple<Resource, Resource, Resource>;
using TriplePattern = std::tuple<Term, Term, Term>;
using VariableMap = std::unordered_map<Variable, Resource>;

// Special constants
const Resource INVALID_RESOURCE = -1;
const Term INVALID_TERM = Term{INVALID_RESOURCE};
const TriplePattern INVALID_PATTERN =
    std::make_tuple(INVALID_TERM, INVALID_TERM, INVALID_TERM);

// Enumerations
enum PatternType {XYZ, SYZ, XPZ, XYO, SPZ, SYO, XPO, SPO};
enum Command {LOAD, SELECT, COUNT, QUIT};
const std::unordered_map<std::string,Command> which_command({
    {"LOAD", Command::LOAD}, {"SELECT", Command::SELECT},
    {"COUNT", Command::COUNT}, {"QUIT", Command::QUIT}
});

// Utility functions
namespace utils {
    PatternType get_pattern_type(TriplePattern);
    Term apply_map(VariableMap, Term);
    std::unordered_set<Variable> get_variables(TriplePattern);
    template <class T> std::unordered_set<T>
        intersect(std::unordered_set<T>, std::unordered_set<T>);
}

// Inject custom hash function for tuples into std
template <class T>
inline void _hash_combine(std::size_t& seed, const T& v) {
    seed ^= std::hash<T>()(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}
template<typename ...TT>
struct std::hash<std::tuple<TT...>> {
    std::size_t operator()(const std::tuple<TT...>& tuple) const noexcept {
        size_t seed = 0;
        std::apply([&](auto ...elems){(_hash_combine(seed, elems),...);},
                   tuple);
        return seed;
    }
};
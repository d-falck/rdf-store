#pragma once

#include <tuple>
#include <unordered_map>
#include <variant>

#include <hash.h>

enum class Variable : char;
enum PatternType {XYZ, SYZ, XPZ, XYO, SPZ, SYO, XPO, SPO};

using Resource = int;
using Term = std::variant<Variable, Resource>;
using ResourcePair = std::tuple<Resource, Resource>;
using ResourceTriple = std::tuple<Resource, Resource, Resource>;
using TriplePattern = std::tuple<Term, Term, Term>;
using VariableMap = std::unordered_map<Variable, Resource>;

Resource INVALID_RESOURCE = -1;
Term INVALID_TERM = Term{INVALID_RESOURCE};
TriplePattern INVALID_PATTERN = std::make_tuple(INVALID_TERM,
                                                INVALID_TERM, INVALID_TERM);

enum Command {LOAD, SELECT, COUNT, QUIT};
std::unordered_map<std::string,Command> which_command({
    {"LOAD", Command::LOAD}, {"SELECT", Command::SELECT},
    {"COUNT", Command::COUNT}, {"QUIT", Command::QUIT}
});
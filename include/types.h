#pragma once
#include <string>
#include <tuple>
#include <unordered_map>
#include <variant>
#include <hash.h>

using Resource = int;
using Variable = std::string;
using Term = std::variant<Variable, Resource>;
using ResourcePair = std::tuple<Resource, Resource>;
using ResourceTriple = std::tuple<Resource, Resource, Resource>;
using TriplePattern = std::tuple<Term, Term, Term>;
using VariableMap = std::unordered_map<Variable, Resource>;

const Resource INVALID_RESOURCE = -1;
const Term INVALID_TERM = Term{INVALID_RESOURCE};
const TriplePattern INVALID_PATTERN = std::make_tuple(INVALID_TERM,
                                                INVALID_TERM, INVALID_TERM);

enum PatternType {XYZ, SYZ, XPZ, XYO, SPZ, SYO, XPO, SPO};
enum Command {LOAD, SELECT, COUNT, QUIT};
const std::unordered_map<std::string,Command> which_command({
    {"LOAD", Command::LOAD}, {"SELECT", Command::SELECT},
    {"COUNT", Command::COUNT}, {"QUIT", Command::QUIT}
});
#include <stdexcept>
#include <string>
#include <tuple>
#include <RDFIndex.h>
#include <utils.h>

RDFIndex::~RDFIndex() {
    int n = _table.size();
    for (int i=0; i<n; i++) delete _table[i];
}

void RDFIndex::add(Resource s, Resource p, Resource o) {
    // Only proceed if not already present
    try {_index_SPO.at(std::make_tuple(s, p, o));}
    catch (std::out_of_range _) {
        // Add new table row and update _index_SPO
        _TableRow* new_row = new _TableRow{s, p, o};
        _table.push_back(new_row);
        _index_SPO[std::make_tuple(s, p, o)] = new_row;
        
        // Update SP-list and _index_SP, _index_S
        try {
            // Insert new_row just after first p-item in SP-list
            _TableRow* row = _index_SP.at(std::make_tuple(s, p));
            new_row->next_SP = row->next_SP;
            row->next_SP = new_row;
        } catch (std::out_of_range _) {
            // Insert new_row at head of SP-list
            new_row->next_SP = _index_S[s]; // Potentially null
            _index_S[s] = new_row;
            _index_SP[std::make_tuple(s,p)] = new_row;
        }
        _len_S[_index_S.at(s)]++;

        // Update OP-list and _index_OP, _index_O
        try {
            // Insert new_row just after first p-item in OP-list
            _TableRow* row = _index_OP.at(std::make_tuple(o, p));
            new_row->next_OP = row->next_OP;
            row->next_OP = new_row;
        } catch (std::out_of_range _) {
            // Insert new_row at head of OP-list
            new_row->next_OP = _index_O[o]; // Potentially null
            _index_S[o] = new_row;
            _index_OP[std::make_tuple(o,p)] = new_row;
        }
        _len_O[_index_O.at(o)]++;

        // Insert new row at head of P-list and update _index_P
        new_row->next_P = _index_P[p]; // Potentially null
        _index_P[p] = new_row;
    }
}

std::function<std::optional<VariableMap>()> RDFIndex::evaluate(
        Term a, Term b, Term c) {
    std::function<bool(_TableRow*)> condition =
        [] (_TableRow* row) {return true;};
    std::function<VariableMap(_TableRow*)> implied_map;
    _TableRow* head;
    std::function<_TableRow*(_TableRow*)> next;

    switch (utils::get_pattern_type(std::make_tuple(a, b, c))) {
        case XYZ: {
            Variable x = std::get<Variable>(a);
            Variable y = std::get<Variable>(b);
            Variable z = std::get<Variable>(c);
            if (x == y && y == z) condition = [] (_TableRow* row) {
                                return row->s == row->p && row->p == row->o;};
            else if (x == y) condition = [] (_TableRow* row) {
                                return row->s == row->p;};
            else if (y == z) condition = [] (_TableRow* row) {
                                return row->p == row->o;};
            else if (x == z) condition = [] (_TableRow* row) {
                                return row->s == row->o;};
            head = _table[0];
            int i = 0;
            next = [=] (_TableRow* row) mutable {
                do {row = _table[i++];}
                    while (row != nullptr && !condition(row));
                return (i <= _table.size()) ? row : nullptr;};
            implied_map = [=] (_TableRow* row) {
                return VariableMap{{x,row->s},{y,row->p},{z,row->o}};};
            break;
        }
        case SYZ: {
            Resource s = std::get<Resource>(a);
            Variable y = std::get<Variable>(b);
            Variable z = std::get<Variable>(c);
            if (y == z) condition =
                [] (_TableRow* row) {return row->p == row->o;};
            head = _index_S[s];
            next = [=] (_TableRow* row) {
                do {row = row->next_SP;}
                    while (row != nullptr && !condition(row));
                return row;};
            implied_map = [=] (_TableRow* row) {
                return VariableMap{{y,row->p},{z,row->o}};};
            break;
        }
        case XYO: {
            Variable x = std::get<Variable>(a);
            Variable y = std::get<Variable>(b);
            Resource o = std::get<Resource>(c);
            if (x == y) condition =
                [] (_TableRow* row) {return row->s == row->p;};
            head = _index_O[o];
            next = [=] (_TableRow* row) {
                do {row = row->next_OP;}
                    while (row != nullptr && !condition(row));
                return row;};
            implied_map = [=] (_TableRow* row) {
                return VariableMap{{x,row->s},{y,row->p}};};
            break;
        }
        case XPZ: {
            Variable x = std::get<Variable>(a);
            Resource p = std::get<Resource>(b);
            Variable z = std::get<Variable>(c);
            if (x == z) condition =
                [] (_TableRow* row) {return row->s == row->o;};
            head = _index_P[p];
            next = [=] (_TableRow* row) {
                do {row = row->next_P;}
                    while (row != nullptr && !condition(row));
                return row;};
            implied_map = [=] (_TableRow* row) {
                return VariableMap{{x,row->s},{z,row->o}};};
            break;
        }
        case SPZ: {
            Resource s = std::get<Resource>(a);
            Resource p = std::get<Resource>(b);
            Variable z = std::get<Variable>(c);
            head = _index_SP[std::make_tuple(s,p)];
            next = [=] (_TableRow* row) {
                row = row->next_SP;
                return (row != nullptr && row->p == p) ? row : nullptr;};
            implied_map =
                [=] (_TableRow* row) {return VariableMap{{z,row->o}};};
            break;
        }
        case XPO: {
            Variable x = std::get<Variable>(a);
            Resource p = std::get<Resource>(b);
            Resource o = std::get<Resource>(c);
            head = _index_OP[std::make_tuple(o,p)];
            next = [=] (_TableRow* row) {
                row = row->next_OP;
                return (row != nullptr && row->p == p) ? row : nullptr;};
            implied_map =
                [=] (_TableRow* row) {return VariableMap{{x,row->s}};};
            break;
        }
        case SYO: {
            Resource s = std::get<Resource>(a);
            Variable y = std::get<Variable>(b);
            Resource o = std::get<Resource>(c);
            if (_len_S[_index_S[s]] >= _len_O[_index_O[o]]) {
                condition = [=] (_TableRow* row) {return row->o == o;};
                head = _index_S[s];
                next = [=] (_TableRow* row) {
                    do {row = row->next_SP;}
                        while (row != nullptr && !condition(row));
                    return row;};
            } else {
                condition = [=] (_TableRow* row) {return row->s == s;};
                head = _index_O[o];
                next = [=] (_TableRow* row) {
                    do {row = row->next_OP;}
                        while (row != nullptr && !condition(row));
                    return row;};
            }
            break;
        }
        case SPO: {
            Resource s = std::get<Resource>(a);
            Resource p = std::get<Resource>(b);
            Resource o = std::get<Resource>(c);
            head = _index_SPO[std::make_tuple(s,p,o)];
            next = [] (_TableRow* row) {return nullptr;};
            implied_map = [=] (_TableRow* row) {return VariableMap{};};
            break;
        }
    }

    _TableRow* current =
        (head == nullptr || condition(head)) ? head : next(head);
    return [=] () mutable {
        if (current == nullptr) return std::optional<VariableMap>();
        else {
            VariableMap map = implied_map(current);
            current = next(current);
            return std::make_optional<VariableMap>(map);
        }
    };
}
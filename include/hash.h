#pragma once
#include <tuple>

// Function for combining two hashes
template <class T>
inline void hash_combine(std::size_t& seed, const T& v) {
    seed ^= std::hash<T>()(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

// Inject custom hash function for tuples into std
template<typename ...TT>
struct std::hash<std::tuple<TT...>> {
    std::size_t operator()(const std::tuple<TT...>& tuple) const noexcept {
        size_t seed = 0;
        std::apply([&](auto ...elems){(hash_combine(seed, elems),...);}, tuple);
        return seed;
    }
};
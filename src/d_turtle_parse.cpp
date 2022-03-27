#include <chrono>
#include <exception>
#include <iostream>
#include <sstream>
#include <tuple>
#include <System.h>
#include <utils.h>

void System::load_triples(std::string str) {
    auto start = std::chrono::high_resolution_clock::now();

    // Split by whitespace
    std::stringstream stream(str);
    std::vector<std::string> words;
    for (std::string word; stream>>word;) words.push_back(word);

    // Check it's the right length
    if (words.size() % 4 != 0)
        throw std::invalid_argument("Invalid N-Triples syntax");

    // Parse each resource
    for (int i=0; i < words.size(); i += 4) {
        if (words[i+3] != ".")
            throw std::invalid_argument("Triples must be separated by periods");
        Resource s = _encode_resource(words[i]);
        Resource p = _encode_resource(words[i+1]);
        Resource o = _encode_resource(words[i+2]);
        _index.add(s, p, o);
    }

    // Print summary
    auto end = std::chrono::high_resolution_clock::now();
    int elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>
        (end-start).count();
    std::cout << words.size()/4 << " triples loaded in "
              << elapsed_ms <<" ms." << std::endl;
}

Resource System::_encode_resource(std::string name) {
    int n = name.length();
    bool valid = (name[0] == '<' && name[n-1] == '>') ||
                 (name[0] == '"' && name[n-1] == '"');
    if (!valid) throw std::invalid_argument(
            "Resources must be enclosed in quotes or angle brackets");
    // Add this resource to our hash map if we haven't seen it before
    if (_resource_ids.count(name) == 0) {
        _stored_resources.push_back(name);
        _resource_ids[name] = _stored_resources.size()-1;
    }
    return _resource_ids[name];
}

std::string System::_decode_resource(Resource id) {
    if (id >= _stored_resources.size())
        throw std::invalid_argument("Resource ID does not exist");
    return _stored_resources[id];
}
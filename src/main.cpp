#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <System.h>
#include <types.h>

int main() {
    System system;
    bool ready = true;

    int debug_loop = 0;

    while (ready) {
        // Wait for command
        std::string keyword, details;
        std::cout << "> ";
        // std::cin >> keyword;
        // std::getline(std::cin, details);

        std::stringstream debug;
        if (debug_loop == 0) debug << "LOAD test.txt";
        else if (debug_loop == 1) debug << "SELECT ?X WHERE { ?X <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> <http://www.w3.org/2002/07/owl#Ontology> . }";
        else debug << "QUIT";
        debug >> keyword;
        std::getline(debug, details);
        debug_loop++;

        // Which command is it?
        Command command;
        try {
            command = which_command.at(keyword);
        } catch (std::out_of_range _) {
            std::cout << "Invalid command." << std::endl;
            continue;
        }

        // Execute the correct command
        try {
            switch (command) {
                case Command::LOAD: {
                    // Strip whitespace
                    std::stringstream ss;
                    ss << details;
                    std::string filename;
                    ss >> filename;

                    // Load from file
                    std::ifstream file(filename);
                    if (!file.is_open())
                        throw std::invalid_argument("Invalid file.");
                    std::stringstream stream;
                    stream << file.rdbuf();

                    // Process triples
                    system.load_triples(stream.str());
                    file.close();
                    break;
                }
                case Command::SELECT: {
                    system.select_query_string(details);
                    break;
                }
                case Command::COUNT: {
                    system.count_query_string(details);
                    break;
                } 
                case Command::QUIT: {
                    ready = false;
                    break;
                }
            }
        } catch (std::invalid_argument e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }
    return 0;
}
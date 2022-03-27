#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <System.h>
#include <utils.h>

int main() {
    System system;
    bool ready = true;

    while (ready) {
        // Wait for command
        std::string keyword, details;
        std::cout << "> ";
        std::cin >> keyword;
        std::getline(std::cin, details);

        // Allow more lines of input if we've seen `{` but not `}`
        if (details.find('{') != details.npos) {
            while (details.find('}') == details.npos) {
                std::string more_details;
                std::getline(std::cin, more_details);
                details.append(" ");
                details.append(more_details);
            }
        }

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
                    system.evaluate_query(details, false); // Print disabled
                    break;
                }
                case Command::COUNT: {
                    system.evaluate_query(details, true); // Print enabled
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
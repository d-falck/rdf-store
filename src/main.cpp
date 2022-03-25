#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <System.h>
#include <types.h>

int main() {
    System system;

    bool ready = true;
    while (ready) {
        // Wait for command
        std::string keyword, command;
        std::cout << "> ";
        std::cin >> keyword;
        std::getline(std::cin, command);

        // Execute according to command
        switch (which_command.at(keyword)) {
            case Command::LOAD: {
                std::ifstream file(command);
                if (!file.is_open()) throw std::invalid_argument("Invalid file.");
                std::stringstream stream;
                stream << file.rdbuf();
                system.load_triples(stream.str());
                file.close();
                break;
            }
            case Command::SELECT: {
                system.select_query_string(command);
                break;
            }
            case Command::COUNT: {
                system.count_query_string(command);
                break;
            } 
            case Command::QUIT: {
                ready = false;
                break;
            }
            default: {
                std::cout << "Invalid command." << std::endl;
            }
        }
    }
    return 0;
}
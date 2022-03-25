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
        std::string keyword, details;
        std::cout << "> ";
        std::cin >> keyword;
        std::getline(std::cin, details);

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
                    std::ifstream file(details);
                    if (!file.is_open())
                        throw std::invalid_argument("Invalid file.");
                    std::stringstream stream;
                    stream << file.rdbuf();
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
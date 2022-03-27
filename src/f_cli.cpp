/**
 * @file f_cli.cpp
 * @author Candidate 1034792
 * @brief Implementation component (f)
 * 
 * The component implementing the command line interface outlined in the paper.
 * Contains the main() function called upon execution of the program.
 */
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <System.h>
#include <utils.h>

/**
 * @brief Main function, called by executable. Invokes CLI.
 * 
 * Immediately displays a command prompt and repeatedly listens for one of
 * four commands:
 *  - `LOAD [file_name]`: Load triples from a Turtle file names `file_name`.
 *          Path should be relative to the directory containing the executable.
 *          Not guaranteed to be atomic.
 *  - `SELECT [rest_of_query]`: Evaluate the supplied BGP SPARQL query,
 *          printing results to stdout.
 *  - `COUNT [rest_of_query]`: Evaluate the supplied BGP SPARQL query,
 *          printing only the *number* of results to stdout.
 *  - `QUIT`: Exit the command line interface and terminate the program.
 * 
 * The `SELECT` and `COUNT` commands support multi-line queries as long as the
 * opening brace occurs on the first line. It should thus be possible to paste
 * a multi-line query from a file into the command line and have it executed.
 * 
 * @return int 0 on successful termination
 */
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
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
 * If the executable is invoked with argument `-o` then all `SELECT` and `COUNT`
 * commands will also print the join order used to stdout.
 * 
 * @return int 0 on successful termination
 */
int main(int argc, char** argv) {
    System system;
    bool output_join_order = (argc > 1 && std::string(argv[1]) == "-o");
    bool ready = true;
    bool loading_triples = false;

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
                        throw std::invalid_argument(
                            "File not found. Check the path and try again.");
                    std::stringstream stream;
                    stream << file.rdbuf();

                    // Process triples
                    loading_triples = true;
                    system.load_triples(stream.str());
                    file.close();
                    loading_triples = false;
                    break;
                }
                case Command::SELECT: {
                    // Print enabled
                    system.evaluate_query(details, true, output_join_order);
                    break;
                }
                case Command::COUNT: {
                    // Print disabled
                    system.evaluate_query(details, false, output_join_order);
                    break;
                } 
                case Command::QUIT: {
                    ready = false;
                    break;
                }
            }
        } catch (std::invalid_argument e) {
            std::cout << "Error: " << e.what() << std::endl;
            if (loading_triples) {
                std::cout << "Input file processing terminated due to error. "
                          << "Please note that any triples already processed "
                          << "may be retained." << std::endl;
                loading_triples = false;
            }
        }
    }
    return 0;
}
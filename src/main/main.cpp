/**
 * 
 */

#include <boost/program_options/variables_map.hpp>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/program_options/errors.hpp>
#include <boost/program_options/option.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/value_semantic.hpp>

#include "routine/checkout.hpp"
#include "routine/create.hpp"
// #include "routine/interactive.hpp"
// #include "routine/setup.hpp"
// #include "routine/upload.hpp"
#include "util/join.hpp"

namespace po = boost::program_options;

const std::string NAME = "warzone-osm-mapmaker";
const std::string GIT_LINK = "https://github.com/PatrickLindemann/warzone-osm-mapmaker";

/**
 * Print the general help message
 */
void help()
{
    std::cout << "Usage: " << NAME << " [action]" << '\n'
              << "Available actions:" << '\n'
              << "  " << "(no action) : Launch the mapmaker in interactive mode" << '\n'
              << "  " << "setup       : Setup the mapmaker for Warzone API usage" << '\n'
              << "  " << "checkout    : Get the file info for an OSM file (.osm, .pbf)" << '\n'
              << "  " << "create      : Create a Warzone map from an OSM file (.osm, .pbf)" << '\n'
              << "  " << "upload      : Upload generated map metadata (.json) to Warzone" << '\n'
              << "More information about the mapmaker can be found here: " << GIT_LINK
              << std::endl;
}

/**
 * Print the title
 */
void title()
{
    std::cout << ""
                << " _       __                                     __  ___                            __            "        << '\n'
                << "| |     / /___ __________  ____  ____  ___     /  |/  /___ _____  ____ ___  ____ _/ /_____  _____"        << '\n'
                << "| | /| / / __ `/ ___/_  / / __ \\/ __ \\/ _ \\   / /|_/ / __ `/ __ \\/ __ `__ \\/ __ `/ //_/ _ \\/ ___/"  << '\n'
                << "| |/ |/ / /_/ / /    / /_/ /_/ / / / /  __/  / /  / / /_/ / /_/ / / / / / / /_/ / ,< /  __/ /    "        << '\n'
                << "|__/|__/\\__,_/_/    /___/\\____/_/ /_/\\___/  /_/  /_/\\__,_/ .___/_/ /_/ /_/\\__,_/_/|_|\\___/_/     "  << '\n'
                << "                                                        /_/                                      "        << std::endl;

}

int main(int argc, char* argv[])
{    
    std::string action;
    std::vector<std::string> extra_options;

    // Positional parameters
    po::positional_options_description positional;
    positional.add("action", 1);
    positional.add("extra-options", -1);

    // Optional parameters
    po::options_description options("Allowed options");
    options.add_options()
        ("action", po::value<std::string>(&action), "action") // Hidden
        ("extra-options", po::value<std::vector<std::string>>(&extra_options)->multitoken(), "extra-options") // Hidden
        ("help,h", "help");

    // Try to parse the parameters
    std::vector<std::string> parameters;
    try
    {
        // Read parameters
        po::parsed_options parsed = po::command_line_parser(argc, argv)
            .allow_unregistered()
            .options(options)
            .positional(positional)
            .run();

        // Store the parsed parameters in the variable map
        po::variables_map vm;
        po::store(parsed, vm);
        po::notify(vm);

        if (action.empty() && vm.count("help"))
        {
            help();
            return 0;
        }

        // Collect unrecognized options to pass on for later use
        parameters = po::collect_unrecognized(
            parsed.options,
            po::exclude_positional
        );
        parameters.insert(parameters.end(), extra_options.begin(), extra_options.end());

        // Check if any arguments were specified
        if (!action.empty())
        {
            // Execute the program according to the action
            boost::algorithm::to_lower(action);
            if (action == "help")
            {
                help();
            }
            else if (action == "setup")
            {
                // routine::setup::run(parameters);
            }
            else if (action == "checkout")
            {
                routine::checkout::run(parameters);
            }
            else if (action == "create")
            {
                routine::create::run(parameters, argv);
            }
            else if (action == "upload")
            {
                // routine::upload::run(parameters);
            }
            else
            {
                // Found unknown action
                throw po::unknown_option("Unknown action '" + action + "'");
            }
            return 0;
        }
        else if (!parameters.empty())
        {
            // Other parameters were specified for the interactive mode, which is not
            // allowed
            throw po::unknown_option("Invalid option(s) for interactive mode : " + util::join(parameters));
        }
    }
    catch (const std::exception& ex)
    {
        std::cout << "[Error] " << ex.what() << std::endl; 
        return 1;
    }

    // No arguments specified, start interactive mode
    // routine::interactive::run(parameters);

    return 0;
}
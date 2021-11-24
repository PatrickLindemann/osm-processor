/*
  Warzone OSM Mapmaker
  Version 1.0.0
  https://github.com/PatrickLindemann/warzone-osm-mapmaker

  Licensed under the MIT License <http://opensource.org/licenses/MIT>.
  SPDX-License-Identifier: MIT
  Copyright (c) 2021 Patrick Lindemann.

  Permission is hereby granted, free of charge, to any  person obtaining a copy
  of this software and associated  documentation files (the "Software"), to deal
  in the Software  without restriction, including without  limitation the rights
  to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
  copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
  furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  
  THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
  IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
  FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
  AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
  LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "routine.hpp"
#include "checkout.hpp"
#include "create.hpp"
#include "prepare.hpp"
#include "setup.hpp"
#include "upload.hpp"

#define DEBUG 1 

namespace po = boost::program_options;

const std::string NAME = "warzone-osm-mapmaker";
const std::string GIT_LINK = "https://github.com/PatrickLindemann/warzone-osm-mapmaker";

/**
 *
 */
const std::unordered_map<std::string, std::shared_ptr<Routine>> ROUTINES{
    {"checkout", std::make_shared<Checkout>(Checkout())},
    {"create",   std::make_shared<Create>(Create())},
    {"prepare",  std::make_shared<Prepare>(Prepare())},
    {"setup",    std::make_shared<Setup>(Setup())},
    {"upload",   std::make_shared<Upload>(Upload())}
};

/**
 * Print the command help message
 */
void help()
{
    std::cout << "Usage: " << NAME << " [command]" << '\n'
              << "Available commands:" << '\n'
              << "  " << "checkout     : Get the file info for an OSM file (.osm, .pbf)" << '\n'
              << "  " << "create       : Create a Warzone map from an OSM file (.osm, .pbf)" << '\n'
              << "  " << "prepare      : Prepare an OSM file (.osm, .pbf) by extracting its boundaries" << '\n'
              << "  " << "setup        : Setup the mapmaker for Warzone API usage" << '\n'
              << "  " << "upload       : Upload generated map metadata (.json) to Warzone" << '\n'
              << "  " << "help         : Shows this help message" << '\n'
              << "More information about the mapmaker can be found here: " << GIT_LINK
              << std::endl;
}

int main(int argc, char* argv[])
{      
    try
    {
        std::string command;

        if (argc > 1)
        {
            // Command needs to be the first argument
            command = argv[1];
            // Check if help flag was specified
            if (command.empty() || command == "-h" || command == "--help" || command == "help")
            {
                help();
                return 0;
            }
            else if (boost::starts_with(command, "-"))
            {
                throw std::invalid_argument("Expected command as first parameter, but found '" + command + "'");
            }
        }

        if (command.empty())
        {
            throw po::unknown_option("No command specified.");
        }

        // Create the routine according to the command
        boost::algorithm::to_lower(command);

        // Retrieve the routine according to the specified command
        if (!ROUTINES.count(command))
        {
            throw po::unknown_option("Unknown command '" + command + "'");
        }
        std::shared_ptr<Routine> routine = ROUTINES.at(command);

        // Execute the routine
        routine->init(argc, argv);
        if (routine->variables().count("help"))
        {
            routine->help();
            return 0;
        }
        routine->setup();
        routine->run();
    }  
    catch (const std::exception& ex)
    {
        std::cout << "[Error] " << ex.what() << std::endl;
        if (DEBUG)
        {
            throw ex;
        }
        return 1;
    }

    return 0;
}
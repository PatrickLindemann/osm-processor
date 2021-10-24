/*
  Warzone OSM Mapmaker
  Version 1.0.0
  https://github.com/PatrickLindemann/warzone-osm-mapmaker

  Licensed under the MIT License <http://opensource.org/licenses/MIT>.
  SPDX-License-Identifier: MIT
  Copyright (c) 2021 Patrick Lindemann.

  Permission is hereby  granted, free of charge, to any  person obtaining a copy
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
#include <stdexcept>
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
#include "routine/setup.hpp"
#include "routine/upload.hpp"
#include "util/join.hpp"

#define DEBUG 1

namespace po = boost::program_options;

const std::string NAME = "warzone-osm-mapmaker";
const std::string GIT_LINK = "https://github.com/PatrickLindemann/warzone-osm-mapmaker";

/**
 * Print the command help message
 */
void help()
{
    std::cout << "Usage: " << NAME << " [command]" << '\n'
              << "Available actions:" << '\n'
              << "  " << "(no command) : Launch the mapmaker in interactive mode" << '\n'
              << "  " << "setup        : Setup the mapmaker for Warzone API usage" << '\n'
              << "  " << "checkout     : Get the file info for an OSM file (.osm, .pbf)" << '\n'
              << "  " << "create       : Create a Warzone map from an OSM file (.osm, .pbf)" << '\n'
              << "  " << "upload       : Upload generated map metadata (.json) to Warzone" << '\n'
              << "  " << "help         : Shows this help message" << '\n'
              << "More information about the mapmaker can be found here: " << GIT_LINK
              << std::endl;
}

int main(int argc, char* argv[])
{       

    std::string command;
    
    /*
    try
    {
    */

        if (argc > 1)
        {
            // Command needs to be the first argument
            command = argv[1];
            // Check if help flag was specified
            if (command == "-h" || command == "--help" || command == "help")
            {
                help();
                return 0;
            }
            else if (boost::starts_with(command, "-"))
            {
                throw std::invalid_argument("Expected command as first parameter, but found '" + command + "'");
            }
        }

        // Execute the program according to the command
        if (!command.empty())
        {
            // Check if command is valid
            boost::algorithm::to_lower(command);

            // Execute command
            if (command == "setup")
            {
                routine::setup::run(argc, argv);
            }
            else if (command == "checkout")
            {
                routine::checkout::run(argc, argv);
            }
            else if (command == "create")
            {
                routine::create::run(argc, argv);
            }
            else if (command == "upload")
            {
                routine::upload::run(argc, argv);
            }
            else
            {
                throw po::unknown_option("Unknown command '" + command + "'");
            }
        }
        else
        {
            // Execute in interactive mode
            // routine::interactive::run(argc, argv);
        }

    /*
    }
    catch (const std::exception& ex)
    {
        std::cout << "[Error] " << ex.what() << std::endl; 
        return 1;
    }
    */

    return 0;
}
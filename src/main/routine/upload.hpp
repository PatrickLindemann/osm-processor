#pragma once

#include "model/config.hpp"
#include <algorithm>
#include <exception>
#include <sstream>
#include <stdexcept>
#include <string>
#include <iostream>
#include <regex>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/program_options.hpp>
#include <boost/program_options/value_semantic.hpp>

#include "io/reader.hpp"
#include "util/validate.hpp"

namespace fs = boost::filesystem;
namespace po = boost::program_options;

namespace routine
{

    namespace upload
    {
        
        /* Main function */

        void run(int argc, char* argv[])
        {
            // Extract file path from argv and remove the command from it
            const fs::path FILE_PATH = fs::system_complete(fs::path(argv[0]));
            const fs::path ROOT_DIR = FILE_PATH.parent_path();
            argc--;
            argv++;

            // Define variables
            fs::path input_path;
            long map_id;
            fs::path config_path;
            
            // Define the positional options
            po::positional_options_description positional;
            positional.add("input", -1);
            positional.add("map-id", -1);

            // Define the general options
            po::options_description options("Allowed options");
            options.add_options()
                ("input", po::value<fs::path>(&input_path),
                    "Sets the input file path.\nAllowed file formats: .json")
                ("map-id", po::value<long>(&map_id),
                        "Sets the map id that the metadata changes will be made to.")
                ("config,c", po::value<fs::path>(&config_path)->default_value(ROOT_DIR / "config.json"),
                    "Sets the path to the configuration file (config.json).")
                ("help,h", "Shows this help message.");

            // Parse the specified arguments
            po::variables_map vm;
            po::store(po::command_line_parser(argc, argv)
                .options(options)
                .positional(positional)
                .run(), vm);
            po::notify(vm);

            // Validate the parsed variables
            // If a variable is invalid, the exception will be passed to the
            // executing instance
            util::validate_file("input", input_path);
            util::validate_id("map-id", map_id);
            util::validate_file("config", config_path);  

            // Read the config file
            model::Config config = io::reader::read_config(config_path.string());
            
            // Read the input metadata file

            // Create the API request
            std::stringstream stream;

            // email: ""
            // APIToken: ""
            // mapID: ""
            // commands: []
            
            // Territory commands:
            // { command: "setTerritoryName", id: territory.id(), name: "territory.name()" }
            // { command: "setTerritoryCenterPoint", id: territory.id(), x: "territory.center().x()", y: "territory.center().y()" }
            // { command: "addTerritoryConnection", id1: territory.id(), id2: neighbor.id(), warp: "Normal" }
            
            // Bonus commands:
            // { command: "addBonus", name: "bonus.name()", armies: bonus.armies(), color: "bonus.color()" }
            // { command: "addTerritoryToBonus", id: child.id(), bonusName: "bonus.name()" }

        }

    }

}
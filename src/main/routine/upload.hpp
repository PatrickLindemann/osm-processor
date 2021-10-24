#pragma once

#include <algorithm>
#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/program_options.hpp>
#include <boost/program_options/value_semantic.hpp>

#include "io/reader.hpp"
#include "model/container.hpp"
#include "model/map/map.hpp"
#include "util/request.hpp"
#include "util/validate.hpp"

namespace fs = boost::filesystem;
namespace po = boost::program_options;

using namespace model;


namespace routine
{

    namespace upload
    {
        
        /* Main function */

        void run(int argc, char* argv[])
        {
            // Extract file path from argv and remove the command from it
            const fs::path FILE_PATH = fs::system_complete(fs::path(argv[0]));
            const fs::path FILE_DIR = FILE_PATH.parent_path();
            argc--;
            argv++;

            // Define variables
            fs::path input_path;
            long map_id;
            fs::path config_path;
            
            // Define the positional options
            po::positional_options_description positional;
            positional.add("input", 1);
            positional.add("map-id", 1);

            // Define the general options
            po::options_description options("Allowed options");
            options.add_options()
                ("input", po::value<fs::path>(&input_path),
                    "Sets the input file path.\nAllowed file formats: .json")
                ("map-id", po::value<long>(&map_id),
                        "Sets the map id that the metadata changes will be made to.")
                ("config,c", po::value<fs::path>(&config_path)->default_value(FILE_DIR / "config.json"),
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
            ConfigContainer config = io::reader::read_config(config_path.string());
            
            // Read the input metadata file
            map::Map map = io::reader::read_metadata(input_path.string()); 

            std::cout << "Preparing the request payload..." << std::endl;
            std::string payload = util::create_payload(map_id, config, map);
            std::cout << payload << std::endl;
            std::cout << "Prepared request data successfully." << std::endl;

            // Send the request
            std::cout << "Sending request for map " << map_id << " to " << "https://www.warzone.com/API/SetMapDetails" << std::endl;
            util::ResponseContainer response = util::post_metadata(payload);
            std::cout << "Received response: " << response.code << " " << response.reason << '\n'
                      << response.body << std::endl;
        }

    }

}
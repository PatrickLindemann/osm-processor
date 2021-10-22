#pragma once

#include <iostream>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/program_options.hpp>
#include <boost/program_options/value_semantic.hpp>

#include "io/writer.hpp"
#include "model/container.hpp"
#include "util/validate.hpp"

namespace fs = boost::filesystem;
namespace po = boost::program_options;

namespace routine
{

    namespace setup
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
            std::string email;
            std::string api_token;
            fs::path outdir;

            // Define the general options
            po::options_description options("Allowed options");
            options.add_options()
                ("email,e", po::value<std::string>(&email),
                    "Sets the Warzone user e-mail")
                ("api-token,t", po::value<std::string>(&api_token),
                    "Sets the Warzone API Token")
                ("outdir,o", po::value<fs::path>(&outdir),
                    "Sets the output directory of the configuration file (config.json)")
                ("help,h", "Shows this help message.");

            // Parse the specified arguments
            po::variables_map vm;
            po::store(po::command_line_parser(argc, argv)
                .options(options)
                .run(), vm);
            po::notify(vm);

            // Set outdir default
            if (outdir.string() != "")
            {   
                util::validate_dir("outdir", outdir);
            }
            else
            {
                outdir = FILE_DIR;
            }

            if (vm.count("help"))
            {
                std::cout << options << std::endl;
                return;
            }

            // Collect email if it was not entered
            if (email.empty())
            {
                std::cout << "Enter your Warzone user e-mail address:" << std::endl;
                std::cin >> email;
            }

            // Collect api token if it was not entered
            if (api_token.empty())
            {
                std::cout << "Enter your Warzone API token:" << std::endl;
                std::cin >> api_token;
            }

            // Create the config object
            model::ConfigContainer config{ email, api_token };

            // Write settings to config.json
            fs::path config_path = outdir / "config.json";
            std::cout << "Writing configuration to " << config_path << "." << std::endl;
            io::writer::write_config(config_path.string(), config);
        }

    }

}
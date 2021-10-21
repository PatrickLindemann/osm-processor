#pragma once

#include "io/writer.hpp"
#include "model/config.hpp"
#include <iostream>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/program_options.hpp>
#include <boost/program_options/value_semantic.hpp>

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
            const fs::path ROOT_DIR = FILE_PATH.parent_path();
            argc--;
            argv++;
    
            // Define constants
            const std::string API_TOKEN_WIKI = "https://www.warzone.com/wiki/Get_API_Token_API";
            const std::string API_TOKEN_LINK = "https://www.warzone.com/API/GetAPIToken";

            // Define variables
            std::string email;
            std::string api_token;

            // Define the general options
            po::options_description options("Allowed options");
            options.add_options()
                ("email,e", po::value<std::string>(&email),
                    "Sets the e-mail")
                ("api-token,t", po::value<std::string>(&api_token),
                    "Sets the api-token")
                ("help,h", "Shows this help message.");

            // Parse the specified arguments
            po::variables_map vm;
            po::store(po::command_line_parser(argc, argv)
                .options(options)
                .run(), vm);
            po::notify(vm);

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
            model::Config config{ email, api_token };

            // Write settings to config.json
            fs::path config_path = fs::canonical(ROOT_DIR) / "config.json";
            std::cout << "Writing configuration to " << config_path << "." << std::endl;
            io::writer::write_config(config_path.string(), config);
        }

    }

}
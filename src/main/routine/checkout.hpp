#pragma once

#include <boost/program_options.hpp>
#include <boost/program_options/value_semantic.hpp>

#include "io/reader.hpp"
#include "model/container.hpp"
#include "model/type.hpp"
#include "util/validate.hpp"

namespace fs = boost::filesystem;
namespace po = boost::program_options;

namespace routine
{

    namespace checkout
    {
        
        /* Main function */

        void run(int argc, char* argv[])
        {
            argc--;
            argv++;

            // Define variables
            fs::path input;

            // Define the positional options
            po::positional_options_description positional;
            positional.add("input", -1);

            // Define the general options
            po::options_description options("Allowed options");
            options.add_options()
                ("input", po::value<fs::path>(&input),
                    "The input OSM file path.\nAllowed file formats: .osm, .pbf")
                ("help,h", "Shows this help message");

            // Parse the specified arguments
            po::variables_map vm;
            po::store(po::command_line_parser(argc, argv)
                .options(options)
                .positional(positional)
                .run(), vm);
            po::notify(vm);

            // Validate the parsed variables. If a variable is invalid,
            // the exception will be passed to the executing instance.
            util::validate_file("input", input);

            // Read the file info and print it to the console
            model::InfoContainer info = io::reader::read_fileinfo(input.string());
            info.print(std::cout);

        }

    }

}
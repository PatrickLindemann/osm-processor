#include <string>
#include <iostream>
#include <exception>
#include <algorithm>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <osmium/memory/buffer.hpp>
#include <boost/program_options.hpp>
#include <boost/program_options/value_semantic.hpp>

#include "mapmaker/model.hpp"
#include "mapmaker/builder.hpp"
#include "io/reader.hpp"
#include "io/writer.hpp"

namespace fs = boost::filesystem;
namespace po = boost::program_options;

static bool verbose;

int main(int argc, char* argv[])
{      
    fs::path input;
    fs::path output;
    int32_t territory_level;
    int32_t bonus_level;
    int32_t width;
    int32_t height;
    double_t epsilon;
    bool cache;

    // Get the directory of the executable
    const fs::path FILE_PATH = fs::system_complete(fs::path(argv[0]));

    // Create the parameters and descriptions
    po::options_description desc("Program options");
    desc.add_options()
        ("input", po::value<fs::path>(&input),
            "sets the input file path\nallowed file formats: .osm, .pbf")
        ("output,o", po::value<fs::path>(&output),
                "sets the target file path")
        ("territory-level,l", po::value<int32_t>(&territory_level)->default_value(6),
            "sets which boundaries should be used as territories."
            "\ninteger between 1 and 12.")
        ("bonus-level,b", po::value<int32_t>(&bonus_level)->default_value(0),
            "sets which boundaries should be used as bonus links."
            "\ninteger between 1 and 12. if set to 0, no bonus links will be created.")
        ("width", po::value<int32_t>(&width)->default_value(1000),
            "sets the generated map width in pixels."
            "\nif set to 0, the width will be determined automatically.")
        ("height", po::value<int32_t>(&height)->default_value(0),
            "sets the generated map height in pixels."
            "\nif set to 0, the height will be determined automatically.")
        ("epsilon", po::value<double_t>(&epsilon)->default_value(0.0),
                "sets the minimum distance threshold between nodes for the"
                "douglas-peucker compression algorithm."
                "\nif set to 0, no compression will be applied.")
        ("cache", po::bool_switch(&cache)->default_value(false),
                "enables caching of pre-processed osm files for faster executions")
        ("verbose,v", po::bool_switch(&verbose)->default_value(false), "sets the verbose flag which enables debug logs")
        ("help,h", "shows this help message")
    ;

    // Parameter positions
    po::positional_options_description p;
    p.add("input", -1);

    // Read parameters
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).
                options(desc).positional(p).run(), vm);
    po::notify(vm);

    // Print help message if help flag was specified
    if (vm.count("help"))
    {
        std::cout << "Usage: pbf-processor <input> [parameters]\n" << desc << std::endl;
        return 0;
    }

    // Verify that input was provided
    if (input.string() == "")
    {
        std::cout << "[Error] " << "No input file specified" << std::endl;
        return 1;
    }
        
    // Validate that input file exists
    fs::path input_path = fs::path(input);
    try
    {
        input_path = fs::canonical(input_path);
    }
    catch (std::exception &ex)
    {
        std::cout << "[Error] " << "Specified input file " << input_path << " does not exist." << std::endl;
        return 1;
    }

    // Validate output path
    // If no output file has been specified, use the input file name
    fs::path output_path;
    if (output.string() != "")
    {
       output_path = fs::path(output);
    }
    else
    {
         output_path = FILE_PATH.parent_path() / "../out" / (input_path.filename().replace_extension(".svg"));
    }
    
    // Validate dimensions
    if (width < 0)
    {
        std::cout << "[Error] " << "Invalid width " << width << " specified. Dimensions have to be greater or equal to 0 (auto)." << std::endl;
        return 1;
    }
    if (height < 0)
    {
        std::cout << "[Error] " << "Invalid height " << height << " specified. Dimensions have to be greater or equal to 0 (auto)." << std::endl;
        return 1;
    }
    if (width == 0 && height == 0)
    {
        std::cout << "[Error] " << "Width and height were both set to 0 (auto). At least one dimension must be set." << std::endl;
        return 1;
    }

    // Validate levels
    if (territory_level < 1 || territory_level > 12)
    {
        std::cout << "[Error] " << "Invalid territory level " << territory_level << " specified. Levels must be integers between 1 and 12." << std::endl;
        return 1;
    }
    if (bonus_level != 0)
    {
        if (bonus_level < 1 || bonus_level > 12)
        {
            std::cout << "[Error] " << "Invalid bonus level " << bonus_level << " specified. Levels must be integers between 1 and 12." << std::endl;
            return 1;
        }
        else if (territory_level >= bonus_level)
        {
            std::cout << "[Error] " << "Bonus level " << bonus_level << " is smaller than territory level " << territory_level << "." << std::endl;
            return 1;
        }
    }

    // Validate epsilon
    if (epsilon < 0)
    {   
        std::cout << "[Error] " << "Specified epsilon " << epsilon << " is invalid. The compression level has to be a value greater than 0." << std::endl;
        return 1;
    }
    
    try
    {   
        // Extract areas with the specified criteria from the input file
        std::vector<mapmaker::model::Boundary> boundaries = io::reader::read_osm(input_path.string(), cache);

        // Build the map
        mapmaker::builder::Builder builder {};
        builder.set_territories(boundaries, territory_level);
        if (bonus_level > 0)
            builder.set_bonus_links(boundaries, bonus_level);
        if (epsilon > 0)
            builder.set_epsilon(epsilon);
        mapmaker::model::Map map = builder.build();

        // Export map as svg
        io::writer::write_svg(output_path.string(), map, width, height);

    } catch (std::exception& ex) {
        std::cerr << "[Error]: " << ex.what() << std::endl;
        std::exit(1);
    }

    return 0;
}
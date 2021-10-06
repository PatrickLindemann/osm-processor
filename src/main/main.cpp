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

#include "io/reader.hpp"
#include "io/writer.hpp"
#include "model/map/map.hpp"
#include "mapmaker/algorithm.hpp"
#include "mapmaker/compressor.hpp"
#include "mapmaker/builder.hpp"
#include "mapmaker/projector.hpp"

namespace fs = boost::filesystem;
namespace po = boost::program_options;

using Clock = std::chrono::high_resolution_clock;
using time_point = std::chrono::time_point<std::chrono::_V2::system_clock, std::chrono::duration<long, std::ratio<1, 1000000000>>>;

int main(int argc, char* argv[])
{      
    fs::path input;
    fs::path output;
    int territory_level;
    std::vector<int> bonus_levels;
    int width;
    int height;
    double epsilon;
    bool verbose;
    bool benchmark;

    // Get the directory of the executable
    const fs::path FILE_PATH = fs::system_complete(fs::path(argv[0]));

    // Create the parameters and descriptions
    po::options_description desc("Program options");
    desc.add_options()
        ("input", po::value<fs::path>(&input),
            "Sets the input file path.\nAllowed file formats: .osm, .pbf")
        ("output,o", po::value<fs::path>(&output),
                "Sets the target file path.\nAllowed file formats: .svg")
        ("territory-level,t", po::value<int>(&territory_level)->default_value(0),
            "Sets the admin_level of boundaries that will be be used as territories."
            "\nInteger between 1 and 12.")
        ("bonus-levels,b", po::value<std::vector<int>>(&bonus_levels)->multitoken(),
            "Sets the admin_level of boundaries that will be be used as bonus links."
            "\nInteger between 1 and 12. If none are specified, no bonus links will be generated.")
        ("width,w", po::value<int>(&width)->default_value(1000),
            "Sets the generated map width in pixels."
            "\nIf set to 0, the width will be determined automatically.")
        ("height,h", po::value<int>(&height)->default_value(0),
            "Sets the generated map height in pixels."
            "\nIf set to 0, the height will be determined automatically.")
        ("epsilon,e", po::value<double>(&epsilon)->default_value(0.0),
                "Sets the minimum distance threshold between points for the Douglas-Peucker compression algorithm."
                "\nIf set to 0, no compression will be applied.")
        ("benchmark", po::bool_switch(&benchmark)->default_value(false), "Enables benchmark mode.")
        ("verbose", po::bool_switch(&verbose)->default_value(false), "Enables verbose logging.")
        ("help,h", "Shows this help message.");

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
        std::cout << "Usage: warzone-osm-mapmaker <input> [parameters]\n" << desc << std::endl;
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

    // Validate territory level
    if (territory_level == 0)
    {
        std::cout << "[Error] " << "No territory level specified. Levels must be integers between 1 and 12." << std::endl;
        return 1;
    }
    else if (territory_level < 1 || territory_level > 12)
    {
        std::cout << "[Error] " << "Invalid territory level " << territory_level << " specified. Levels must be integers between 1 and 12." << std::endl;
        return 1;
    }

    // Validate bonus levels
    if (!bonus_levels.empty())
    {
        for (auto& bonus_level : bonus_levels)
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
    }

    // Validate map dimensions
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

    // Validate epsilon
    if (epsilon < 0)
    {   
        std::cout << "[Error] " << "Specified epsilon " << epsilon << " is invalid. The compression level has to be a value greater than 0." << std::endl;
        return 1;
    }
    
    /*
    try
    {
    */  
        // Print the title
        std::cout << ""
                  << " _       __                                     __  ___                            __            "        << '\n'
                  << "| |     / /___ __________  ____  ____  ___     /  |/  /___ _____  ____ ___  ____ _/ /_____  _____"        << '\n'
                  << "| | /| / / __ `/ ___/_  / / __ \\/ __ \\/ _ \\   / /|_/ / __ `/ __ \\/ __ `__ \\/ __ `/ //_/ _ \\/ ___/"  << '\n'
                  << "| |/ |/ / /_/ / /    / /_/ /_/ / / / /  __/  / /  / / /_/ / /_/ / / / / / / /_/ / ,< /  __/ /    "        << '\n'
                  << "|__/|__/\\__,_/_/    /___/\\____/_/ /_/\\___/  /_/  /_/\\__,_/ .___/_/ /_/ /_/\\__,_/_/|_|\\___/_/     "  << '\n'
                  << "                                                        /_/                                      "        << std::endl;

        // Create the vector for time measurments
        std::vector<time_point> times;

        // Read the ile headers 
        io::reader::FileInfo info = io::reader::get_fileinfo(input_path.string());
        info.print(std::cout);

        // TODO: if interactive mode: Ask for territory level and bonus level

        // Read the file contents and extract the nodes, areas and graph
        std::cout << "Reading file and assembling areas." << std::endl;
        io::reader::FileData data = io::reader::get_data(input_path.string(), territory_level, bonus_levels);
        if (!data.incomplete_relations.empty()) {
            std::cerr << "Warning! Some member ways missing for these multipolygon relations:";
            for (const auto id : data.incomplete_relations) {
                std::cerr << " " << id;
            }
            std::cerr << "\n";
        }
        std::cout << "Finished file reading." << std::endl;

        // TODO: if interactive mode: Ask for compression level

        // (-> Not the value, but provide multiple opts "NONE, HIGH, MEDIUM, LOW, ETC.")

        // Compress the Buffers & Graph
        if (epsilon > 0)
        {
            std::cout << "Compressing area edges... " << std::endl;
            size_t nodes_before = data.node_buffer.size();
            mapmaker::compressor::compress(data.node_buffer, data.area_buffer, data.graph, epsilon);
            size_t nodes_after = data.node_buffer.size();
            std::cout << "Compressed areas successfully." << '\n'
                      << "Results: " << '\n'
                      << "  Nodes (before): " << std::to_string(nodes_before) << '\n'
                      << "  Nodes (after):  " << std::to_string(nodes_after) << std::endl;
        }

        //
        std::cout << "Building the map... " << std::endl;
        mapmaker::builder::Config config{ width, height, territory_level, bonus_levels };
        mapmaker::builder::Builder builder{ data.area_buffer, data.node_buffer, data.graph, config };
        map::Map map = builder.build();
        std::cout << "Finished build sucessfully." << std::endl;

        auto [components, vertex_component_list] = mapmaker::algorithm::get_components(data.graph);
        
        // Export map as svg
        std::cout << "Exporting data to " << output_path << "..." << std::endl;
        io::writer::write_svg(output_path.string(), map);
        std::cout << "Data export finished." << std::endl;

    /*
    }
    catch (std::exception& ex) {
        std::cerr << "[Error]: " << ex.what() << std::endl;
        std::exit(1);
    }
    */

    return 0;
}
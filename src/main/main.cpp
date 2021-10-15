#include <string>
#include <iostream>
#include <exception>
#include <algorithm>
#include <unordered_map>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/program_options.hpp>
#include <boost/program_options/value_semantic.hpp>

#include <osmium/memory/buffer.hpp>

#include "io/reader.hpp"
#include "io/writer.hpp"
#include "functions/project.hpp"
#include "mapmaker/assembler.hpp"
#include "mapmaker/builder.hpp"
#include "mapmaker/compressor.hpp"
#include "mapmaker/connector.hpp"
#include "mapmaker/converter.hpp"
#include "mapmaker/inspector.hpp"
#include "mapmaker/projector.hpp"
#include "model/container.hpp"

namespace fs = boost::filesystem;
namespace po = boost::program_options;

using Clock = std::chrono::high_resolution_clock;
using time_point = std::chrono::time_point<std::chrono::_V2::system_clock, std::chrono::duration<long, std::ratio<1, 1000000000>>>;

fs::path input;
fs::path output;
unsigned short territory_level;
std::vector<unsigned short> bonus_levels;
unsigned int width;
unsigned int height;
double epsilon;
bool verbose;
bool benchmark;

void add_options(po::options_description& description)
{
    description.add_options()
        ("input", po::value<fs::path>(&input),
            "Sets the input file path.\nAllowed file formats: .osm, .pbf")
        ("output,o", po::value<fs::path>(&output),
                "Sets the target file path.\nAllowed file formats: .svg")
        ("territory-level,t", po::value<unsigned short>(&territory_level)->default_value(0),
            "Sets the admin_level of boundaries that will be be used as territories."
            "\nInteger between 1 and 12.")
        ("bonus-levels,b", po::value<std::vector<unsigned short>>(&bonus_levels)->multitoken(),
            "Sets the admin_level of boundaries that will be be used as bonus links."
            "\nInteger between 1 and 12. If none are specified, no bonus links will be generated.")
        ("width,w", po::value<unsigned int>(&width)->default_value(1000),
            "Sets the generated map width in pixels."
            "\nIf set to 0, the width will be determined automatically.")
        ("height,h", po::value<unsigned int>(&height)->default_value(0),
            "Sets the generated map height in pixels."
            "\nIf set to 0, the height will be determined automatically.")
        ("epsilon,e", po::value<double>(&epsilon)->default_value(0.0),
                "Sets the minimum distance threshold between points for the Douglas-Peucker compression algorithm."
                "\nIf set to 0, no compression will be applied.")
        ("benchmark", po::bool_switch(&benchmark)->default_value(false), "Enables benchmark mode.")
        ("verbose", po::bool_switch(&verbose)->default_value(false), "Enables verbose logging.")
        ("help,h", "Shows this help message.");
}

template<typename StreamType>
void print_title(StreamType& stream)
{
    // Print the title
    stream << ""
           << " _       __                                     __  ___                            __            "        << '\n'
           << "| |     / /___ __________  ____  ____  ___     /  |/  /___ _____  ____ ___  ____ _/ /_____  _____"        << '\n'
           << "| | /| / / __ `/ ___/_  / / __ \\/ __ \\/ _ \\   / /|_/ / __ `/ __ \\/ __ `__ \\/ __ `/ //_/ _ \\/ ___/"  << '\n'
           << "| |/ |/ / /_/ / /    / /_/ /_/ / / / /  __/  / /  / / /_/ / /_/ / / / / / / /_/ / ,< /  __/ /    "        << '\n'
           << "|__/|__/\\__,_/_/    /___/\\____/_/ /_/\\___/  /_/  /_/\\__,_/ .___/_/ /_/ /_/\\__,_/_/|_|\\___/_/     "  << '\n'
           << "                                                        /_/                                      "        << std::endl;
}

int main(int argc, char* argv[])
{      
    // Get the directory of the executable
    const fs::path FILE_PATH = fs::system_complete(fs::path(argv[0]));

    // Create the parameters and descriptions
    po::options_description desc("Program options");
    add_options(desc);

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

    // Validate inputs
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
         output_path = FILE_PATH.parent_path() / "../out" / (input_path.filename().replace_extension(""));
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
            else if (bonus_level >= territory_level)
            {
                std::cout << "[Error] " << "Bonus level " << bonus_level << " is greater or equal than territory level " << territory_level << "." << std::endl;
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

    print_title(std::cout);

    // TODO: interactive mode

    // Create the vector for time measurments
    std::vector<time_point> times;

    // Prepare the data containers
    model::InfoContainer<double> info; 
    model::DataContainer<double> data;
    data.territory_level = territory_level;
    data.bonus_levels = bonus_levels;

    /* Read the file headers */
    io::reader::get_info(info, input_path.string());
    info.print(std::cout);

    /* Read the file contents and extract the nodes, ways and relations */
    std::cout << "Reading file data from file \"" << input_path << "\"..." << std::endl;
    io::reader::get_data(data, input_path.string(), territory_level, bonus_levels);
    if (!data.incomplete_relations.empty()) {
        std::cerr << "Warning! Some member ways missing for these multipolygon relations:";
        for (const auto id : data.incomplete_relations) {
            std::cerr << " " << id;
        }
        std::cerr << "\n";
    }
    std::cout << "Finished file reading." << std::endl;

    /* Compress the extracted ways */
    if (epsilon > 0)
    {
        std::cout << "Compressing ways... " << std::endl;
        size_t nodes_before = data.nodes.size();
        mapmaker::compressor::Compressor compressor{ data.nodes, data.ways, epsilon };
        compressor.run();
        size_t nodes_after = data.nodes.size();
        std::cout << "Compressed ways successfully." << '\n'
                    << "Results: " << '\n'
                    << "  Nodes (before): " << std::to_string(nodes_before) << '\n'
                    << "  Nodes (after):  " << std::to_string(nodes_after) << std::endl;
    }

    /* Normalize the extracted areas */
    std::cout << "Assembling areas from relations..." << std::endl;    
    mapmaker::assembler::Assembler<double> assembler{ data.nodes, data.ways, data.relations };
    assembler.run();
    data.areas = assembler.areas();
    std::cout << "Assembled areas successfully." << '\n'
                << "Results: " << '\n'
                << "  Relations (before): " << std::to_string(data.relations.size()) << '\n'
                << "  Areas (after):      " << std::to_string(data.areas.size()) << std::endl;

    /* Retrieve the relations for the areas */
    std::cout << "Calculating area relations (neighbors, components, subareas)..." << std::endl;
    mapmaker::inspector::Inspector<double> inspector{ data.nodes, data.ways, data.areas };
    inspector.run();
    data.neighbors = inspector.neighbors();
    data.components = inspector.components();
    data.subareas = inspector.subareas();
    std::cout << "Calculated relations sucessfully. " << std::endl;

    /* Connect islands */
    std::cout << "Connecting islands..." << std::endl;
    std::cout << "Created " << 0 << " new connections." << std::endl;

    /* Apply the map projections */
    std::cout << "Applying the map projections... " << std::endl;
    mapmaker::projector::Projector projector{ data.nodes };  
    // Convert the map coordinates to radians
    projector.apply(functions::RadianProjection<double>{});
    // Apply the MercatorProjection
    projector.apply(functions::MercatorProjection<double>{});
    std::cout << "Applied projections sucessfully on " << data.nodes.size() << " nodes." << std::endl;

    /* Scale the map */
    std::cout << "Scaling the map... " << std::endl;
    geometry::Rectangle<double> bounds = projector.bounds();
    // Check if a dimension is set to auto and calculate its value
    // depending on the map bounds
    if (width == 0 || height == 0)
    {
        if (width == 0)
        {
            width = bounds.width() / bounds.height() * height;
        }
        else
        {
            height = bounds.height() / bounds.width() * width;
        }
    }
    data.width = width;
    data.height = height;
    // Apply the scaling projections
    // Scale the map according to the dimensions
    projector.apply(functions::UnitProjection<double>{
        { bounds.min.x, bounds.max.x },
        { bounds.min.y, bounds.max.y }
    });
    projector.apply(functions::IntervalProjection<double>{
        { 0.0, 1.0 }, { 0.0, 1.0 }, { 0.0, width }, { 0.0, height }
    });
    std::cout << "Scaled the map sucessfully. The output size will be " << width << "x" << height << "px" << std::endl;

    // Assemble the geometries
    std::cout << "Converting areas to geometries... " << std::endl;
    mapmaker::converter::GeometryConverter converter{ data.nodes, data.areas };
    converter.run();
    data.geometries = converter.geometries();
    std::cout << "Built geometries successfully. " << std::endl;

    // Calculate the centerpoints
    std::cout << "Calculating centerpoints... " << std::endl;
    std::cout << "Calculated centerpoints successfully. " << std::endl;

    // Build the final map
    std::cout << "Building the map... " << std::endl;
    mapmaker::builder::MapBuilder builder{ data };
    builder.run();
    map::Map map = builder.map();
    std::cout << "Built the map sucessfully." << std::endl;

    // Export the map data as .svg file
    std::cout << "Exporting map data..." << std::endl;
    std::string outpath_string = output_path.string();
    io::writer::write_metadata(outpath_string, map);
    std::cout << "Exported metadata to " << output_path << ".json" << std::endl;
    io::writer::write_map(outpath_string, map);
    std::cout << "Exported map to " << output_path << ".svg" << std::endl;
    io::writer::write_preview(outpath_string, map);
    std::cout << "Exported metadata to " << output_path << ".preview.svg" << std::endl;
    std::cout << "Data export finished successfully. " << std::endl;

    std::cout << "Finished Mapmaker after " << 0 << " seconds." << std::endl;
    return 0;
}
#include <exception>
#include <iostream>
#include <algorithm>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/program_options.hpp>

#include <arrow/api.h>
#include <arrow/io/api.h>
#include <parquet/arrow/reader.h>
#include <parquet/arrow/writer.h>
#include <parquet/exception.h>

#include <osmium/memory/buffer.hpp>

#include "extractor.hpp"

namespace fs = boost::filesystem;
namespace po = boost::program_options;

static bool verbose;

int main(int argc, char* argv[])
{      
    std::string input;
    std::string output;
    std::string boundary_type;
    double compression_level;

    // Get the directory of the executable
    const fs::path FILE_PATH = fs::system_complete(fs::path(argv[0]));

    // Create the parameters and descriptions
    po::options_description desc("Program options");
    desc.add_options()
        ("input", po::value<std::string>(&input),
            "sets the input file path\nallowed file formats: .pbf, .osm")
        ("output,o", po::value<std::string>(&output),
                "sets the target directory")
        ("boundary-type,b", po::value<std::string>(&boundary_type)->default_value("administrative"),
                "sets the osm boundary type that will be filtered for")
        ("compression-level,c", po::value<double>(&compression_level)->default_value(1.0),
                "sets the minimum distance threshold between nodes for the douglas-peucker compression algorithm")
        // ("cache,c", po::bool_switch(&cache_flag)->default_value(false),
        //        "sets the cache flag, which enables caching of pre-processed files for faster executions")
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
    if (input == "")
    {
        std::cerr << "[Error] No input file specified" << std::endl;
        return 1;
    }
    // Verify input extension
    std::string input_extension = fs::extension(input);
    boost::algorithm::to_lower(input_extension);
    if (!(input_extension == ".pbf" || input_extension == ".osm"))
    {
        std::cerr << "[Error] Invalid input file format. Allowed formats are: .pbf .osm" << std::endl;
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
        std::cerr << "[Error] Specified input file " << input_path << " does not exist" << std::endl;
        return 1;
    }

    // If no output file has been specified, use the input file name
    fs::path output_path;
    if (output != "")
    {
       output_path = fs::path(output);
    }
    else
    {
         output_path = FILE_PATH.parent_path() / "../out" / (input_path.filename().replace_extension(".parquet"));
    }

    // Validate boundary type
    boost::algorithm::to_lower(boundary_type);
    if (
        !(
        boundary_type == "administrative"
            || boundary_type == "aboriginal_lands" 
            || boundary_type == "hazard" 
            || boundary_type == "maritime" 
            || boundary_type == "marker" 
            || boundary_type == "national_park" 
            || boundary_type == "political" 
            || boundary_type == "postal_code" 
            || boundary_type == "special_economic_zone"
            || boundary_type == "protected_area"
        )
    )
    {
        std::cerr << "[Error] Specified boundary type " << boundary_type << " is invalid. Valid values are:\n"
                    "aboriginal_lands\n"
                    "administrative\n"
                    "hazard\n"
                    "maritime\n"
                    "marker\n"
                    "national_park\n"
                    "political\n"
                    "postal_code\n"
                    "special_economic_zone\n"
                    "protected_area" << std::endl;
        return 1;
    }

    // Validate compression_level
    if (compression_level < 0)
    {   
        std::cerr << "[Error] Specified compression_level " << compression_level << " is invalid. The compression level has to be a value greater than 0." << std::endl;
        return 1;
    }
    
    // Process input
    std::shared_ptr<arrow::Table> table;
    try
    {   
        // Extract areas with the specified criteria from the input file
        table = AreaExtractor::run(input_path.string(), boundary_type, compression_level);
    } catch (std::exception& ex) {
        std::cerr << "ERROR IN MAIN: " << ex.what() << std::endl;
        std::exit(1);
    }

    // TODO: Result should be an apache arrow table which can be written to a file here
    std::shared_ptr<arrow::io::FileOutputStream> outfile;
    PARQUET_ASSIGN_OR_THROW(
        outfile,
        arrow::io::FileOutputStream::Open(output_path.string())
    );

    // The last argument to the function call is the size of the RowGroup in
    // the parquet file. Normally you would choose this to be rather large but
    // for the example, we use a small value to have multiple RowGroups.
    PARQUET_THROW_NOT_OK(
        parquet::arrow::WriteTable(*table, arrow::default_memory_pool(), outfile, 3)
    );

    return 0;
}
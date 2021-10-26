#pragma once

#include <algorithm>
#include <exception>
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
#include "io/writer.hpp"
#include "functions/project.hpp"
#include "mapmaker/assembler.hpp"
#include "mapmaker/builder.hpp"
#include "mapmaker/compressor.hpp"
#include "mapmaker/connector.hpp"
#include "mapmaker/filter.hpp"
#include "mapmaker/inspector.hpp"
#include "mapmaker/transformer.hpp"
#include "mapmaker/calculator.hpp"
#include "model/container.hpp"
#include "model/geometry/rectangle.hpp"
#include "util/validate.hpp"
#include "util/title.hpp"

namespace fs = boost::filesystem;
namespace po = boost::program_options;

namespace routine
{

    namespace create
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
            fs::path input;
            fs::path outdir;
            level_type territory_level;
            std::vector<level_type> bonus_levels;
            int width;
            int height;
            double compression_epsilon;
            double filter_epsilon;
            bool verbose;

            // Define the positional options
            po::positional_options_description positional;
            positional.add("input", -1);

            // Define the general options
            po::options_description options("Allowed options");
            options.add_options()
                ("input", po::value<fs::path>(&input),
                    "Sets the input file path.\nAllowed file formats: .osm, .pbf")
                ("outdir,o", po::value<fs::path>(&outdir),
                        "Sets the output folder for the generated map files.")
                ("territory-level,t", po::value<level_type>(&territory_level)->default_value(0),
                    "Sets the admin_level of boundaries that will be be used as territories."
                    "\nInteger between 1 and 12.")
                ("bonus-levels,b", po::value<std::vector<level_type>>(&bonus_levels)->multitoken(),
                    "Sets the admin_level of boundaries that will be be used as bonus links."
                    "\nInteger between 1 and 12. If none are specified, no bonus links will be generated.")
                ("width", po::value<int>(&width)->default_value(1000),
                    "Sets the generated map width in pixels."
                    "\nIf set to 0, the width will be determined automatically with the height.")
                ("height", po::value<int>(&height)->default_value(0),
                    "Sets the generated map height in pixels."
                    "\nIf set to 0, the height will be determined automatically with the width.")
                ("compression-tolerance,c", po::value<double>(&compression_epsilon)->default_value(0.0),
                        "Sets the minimum distance tolerance for the compression algorithm."
                        "\nIf set to 0, no compression will be applied.")
                ("filter-tolerance,f", po::value<double>(&filter_epsilon)->default_value(0.0),
                        "Sets the surface area ratio tolerance for filtering boundaries."
                        "\nIf set to 0, no filter will be applied.")
                ("verbose", po::bool_switch(&verbose)->default_value(false), "Enables verbose logging.")
                ("help,h", "Shows this help message.");

            // Parse the specified arguments
            po::variables_map vm;
            po::store(po::command_line_parser(argc, argv)
                .options(options)
                .positional(positional)
                .run(), vm);
            po::notify(vm);

            // Set outdir default
            if (outdir.string() != "")
            {   
                util::validate_dir("outdir", outdir);
            }
            else
            {
                fs::create_directory(FILE_DIR / "out");
                outdir = fs::canonical(FILE_DIR / "/out");
            }

            // Validate the parsed variables. If a variable is invalid,
            // the exception will be passed to the executing instance.
            util::validate_file("input", input);            
            util::validate_levels(territory_level, bonus_levels);
            util::validate_dimensions(width, height);
            util::validate_epsilon("compression-tolerance", compression_epsilon);
            util::validate_epsilon("filter-tolerance", filter_epsilon);

            util::print_title(std::cout);

            // Sort bonus levels
            std::sort(bonus_levels.begin(), bonus_levels.end());

            // Read the file info and print it to the console
            InfoContainer info = io::reader::read_fileinfo(input.string());
            info.print(std::cout);
            
            // If the territory level was set to auto, choose level with the most
            // boundaries
            if (territory_level == 0)
            {
                auto [l, c] = *std::max_element(info.level_counts.cbegin(), info.level_counts.cend(),
                    [](const std::pair<short, size_t>& e1, const std::pair<short, size_t>& e2)
                    {
                    return e1.second < e2.second;
                    }
                );
                territory_level = l;
            }

            /* Read the file contents and extract the nodes, ways and relations */
            std::cout << "Reading file data from file " << input << "..." << std::endl;
            DataContainer data = io::reader::read_filedata(input.string(), territory_level, bonus_levels);
            if (!data.incomplete_relations.empty()) {
                std::cerr << "Warning! Some member ways missing for these multipolygon relations:";
                for (const auto id : data.incomplete_relations) {
                    std::cerr << " " << id;
                }
                std::cerr << "\n";
            }
            std::cout << "Finished file reading." << std::endl;

            // Compress the extracted ways
            if (compression_epsilon > 0)
            {
                std::cout << "Compressing ways... " << std::endl;
                size_t nodes_before = data.nodes.size();
                mapmaker::compressor::Compressor compressor{ data.nodes, data.ways };
                compressor.compress_ways(compression_epsilon);
                size_t nodes_after = data.nodes.size();
                std::cout << "Compressed ways successfully." << '\n'
                            << "Results: " << '\n'
                            << "  Nodes (before): " << std::to_string(nodes_before) << '\n'
                            << "  Nodes (after):  " << std::to_string(nodes_after) << std::endl;
            }

            /*
            // Assemble the territory areas
            std::cout << "Assembling territories from relations..." << std::endl;    
            mapmaker::assembler::SimpleAreaAssembler territory_assembler{ data.nodes, data.ways, data.relations };
            data.areas = territory_assembler.assemble_areas({ territory_level });
            std::cout << "Assembled " << data.areas.size() << " territories successfully." << std::endl;

            // Create the neighbor graph and component map
            std::cout << "Calculating territory relations (neighbors and components)..." << std::endl;
            mapmaker::inspector::NeighborInspector neighbor_inspector{ data.areas };
            auto [neighbors, components] = neighbor_inspector.get_relations();
            std::cout << "Calculated relations sucessfully. " << std::endl;

            // Apply the area filter on the territories
            if (filter_epsilon > 0)
            {
                std::cout << "Fitering territories by their relative size..." << std::endl;
                mapmaker::filter::AreaFilter territory_filter(
                    data.areas,
                    data.relations,
                    neighbors,
                    components,
                    data.nodes,
                    data.ways
                );
                size_t territories_before = data.areas.size();
                territory_filter.filter_areas(filter_epsilon);
                size_t territories_after = data.areas.size();
                std::cout << "Compressed territories successfully." << '\n'
                            << "Results: " << '\n'
                            << "  Territories (before): " << std::to_string(territories_before) << '\n'
                            << "  Territories (after):  " << std::to_string(territories_after) << std::endl;
            }
            
            // Assemble the bonus areas
            if (!bonus_levels.empty())
            {
                std::cout << "Assembling bonus areas from relations..." << std::endl;    
                mapmaker::assembler::ComplexAreaAssembler bonus_assembler{ data.nodes, data.ways, data.relations };
                size_t areas_before = data.areas.size();
                bonus_assembler.assemble_areas(data.areas, bonus_levels);
                size_t areas_after = data.areas.size();
                std::cout << "Assembled " << areas_after - areas_before << " bonus areas successfully." << std::endl;            
            }

            // Apply the map projections
            std::cout << "Applying the map projections... " << std::endl;
            mapmaker::transformer::Transformer<double> transformer{ data.nodes };  
            // Convert the map coordinates to radians
            transformer.apply(functions::RadianProjection<double>{});
            // Apply the MercatorProjection
            transformer.apply(functions::MercatorProjection<double>{});
            // Apply the MirrorProjection
            // transformer.apply(functions::ScaleTranformation<double>{ 1.0, -1.0 });
            std::cout << "Applied projections sucessfully on " << data.nodes.size() << " nodes." << std::endl;

            // Scale the map
            std::cout << "Scaling the map... " << std::endl;
            geometry::Rectangle<double> bounds = transformer.get_bounds(data.areas);
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
            // Apply the scaling projections
            // Scale the map according to the dimensions
            transformer.apply(functions::UnitProjection<double>{
                { bounds.min().x(), bounds.max().x() },
                { bounds.min().y(), bounds.max().y() }
            });
            transformer.apply(functions::IntervalProjection<double>{
                { 0.0, 1.0 }, { 0.0, 1.0 }, { 0.0, width }, { 0.0, height }
            });
            bounds = { 0.0, 0.0, (double) width, (double) height };
            std::cout << "Scaled the map sucessfully. The output size will be " << width << "x" << height << "px" << std::endl;

            // Assemble the geometries
            std::cout << "Converting areas to geometries... " << std::endl;
            mapmaker::builder::MapBuilder map_builder{ data.nodes, data.areas, neighbors };
            std::string name = std::regex_replace(
                input.filename().string(),
                std::regex("(\\.osm|\\.pbf)"),
                ""
            );
            map::Map map = map_builder.build_map(name, width, height, territory_level, bonus_levels);
            std::cout << "Built geometries successfully. " << std::endl;

            // Calculate the centerpoints
            std::cout << "Calculating centerpoints... " << std::endl;
            mapmaker::calculator::CenterCalculator center_calculator{ map.territories() };
            center_calculator.create_centerpoints();
            std::cout << "Calculated centerpoints successfully. " << std::endl;
           
            // Calculate connections
            if (!bonus_levels.empty())
            {
                // Create the bonus hirarchy
                std::cout << "Calculating Hirarchy..." << std::endl;
                mapmaker::connector::HirarchyCreator hirarchy_creator{
                    map.territories(),
                    map.bonuses(),
                    map.super_bonuses()
                };
                hirarchy_creator.create_hirarchy();
                std::cout << "Calculated hirarchy successfully. " << std::endl;

                // Calculate armies
                std::cout << "Calculating army distribution..." << std::endl;
                mapmaker::calculator::ArmyCalculator army_calculator{
                    map.territories(),
                    map.bonuses(),
                    map.super_bonuses(),
                    neighbors
                };
                army_calculator.calculate_armies(1, 10);
                std::cout << "Calculated armies sucessfully" << std::endl;
            }


            // Calculate connections

            // Export the map data as .svg file
            std::cout << "Exporting map data..." << std::endl;
            
            fs::path outfile_svg = outdir / fs::path(name).replace_extension(".svg");
            io::writer::write_map(outfile_svg.string(), map);
            std::cout << "Exported map to " << outfile_svg << std::endl;
            
            fs::path outfile_json = outdir / fs::path(name).replace_extension(".json");
            io::writer::write_metadata(outfile_json.string(), map);
            std::cout << "Exported metadata to " << outfile_json << std::endl;

            std::cout << "Data export finished successfully. " << std::endl;

            std::cout << "Finished Mapmaker after " << 0 << " seconds." << std::endl;
            */
        }

    }

}
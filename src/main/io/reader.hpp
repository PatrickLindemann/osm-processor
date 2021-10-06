#pragma once

#include <set>
#include <tuple>
#include <string>
#include <algorithm>

#include <osmium/visitor.hpp>
#include <osmium/io/any_input.hpp>
#include <osmium/io/any_output.hpp>
#include <osmium/io/file_format.hpp>
#include <osmium/index/map/flex_mem.hpp>
#include <osmium/tags/matcher.hpp>
#include <osmium/area/assembler.hpp>
#include <osmium/area/multipolygon_manager.hpp>
#include <osmium/handler/node_locations_for_ways.hpp>

#include <boost/lexical_cast.hpp>

#include "model/memory/buffer.hpp"
#include "model/graph/undirected_graph.hpp"
#include "handler/count_handler.hpp"
#include "handler/area_handler.hpp"

using namespace model;

namespace io
{

    namespace reader
    {
        
        /* Definitions */
        
        using NodeBuffer  = memory::Buffer<memory::Node>;
        using AreaBuffer  = memory::Buffer<memory::Area>;
        using Graph       = graph::UndirectedGraph;

        /**
         * 
         */
        struct FileInfo
        {
            /* Members */

            // File information
            std::string name;
            osmium::io::file_format format;
            osmium::io::file_compression compression;
            size_t size;

            // Osmium object information
            size_t nodes;
            size_t ways;
            size_t relations;
            
            // Boundary information
            size_t boundaries;
            std::map<std::string, size_t> levels;

            /* Methods */

            /**
             * @param stream
             */
            template <typename StreamType>
            void print(StreamType& stream) const
            {
                stream << "File:" << '\n'
                    << "  " << "Name: " << name << '\n'
                    << "  " << "Format: " << format << '\n'
                    << "  " << "Compression: " << compression << '\n'
                    << "  " << "Size: " << size << '\n'
                    << "Objects:" << '\n'
                    << "  " << "Nodes: " << nodes << '\n'
                    << "  " << "Ways: " << ways << '\n'
                    << "  " << "Relations: " << relations << '\n'
                    << "Boundaries: " << '\n'
                    << "  " << "Total: " << boundaries << '\n'
                    << "  " << "Levels: " << '\n';
                for (auto& [level, count] : levels)
                {
                    stream << "    " << level << ": " << count << '\n';
                }
                stream << std::endl;
            };

        };

        /**
         * 
         * @param file_path
         * @returns
         */
        FileInfo get_fileinfo(const std::string& file_path)
        {
            // The Reader is initialized here with an osmium::io::File, but could
            // also be directly initialized with a file name.
            osmium::io::File input_file{ file_path };
            osmium::io::Reader reader{ input_file };

            // Create the CountHandler that counts the total number of nodes,
            // ways and relations
            handler::CountHandler count_handler;

            // Create a TagCountHandler that counts the levels for administrative
            // boundaries
            handler::TagValueCountHandler level_handler{ "admin_level" };

            // Apply the counters to the input file
            osmium::apply(reader, count_handler, level_handler);

            // You do not have to close the Reader explicitly, but because the
            // destructor can't throw, you will not see any errors otherwise.
            reader.close();

            // Return results
            return FileInfo{
                file_path,
                input_file.format(),
                input_file.compression(),
                input_file.buffer_size(),
                count_handler.node_count(),
                count_handler.way_count(),
                count_handler.relation_count(),
                level_handler.total(),
                level_handler.counts()
            };
        }

        /**
         * 
         */
        struct FileData
        {
            NodeBuffer node_buffer;
            AreaBuffer area_buffer;
            Graph graph;
            std::vector<osmium::object_id_type> incomplete_relations;
        };

        // The type of index used. This must match the include file above
        using index_type = osmium::index::map::FlexMem<osmium::unsigned_object_id_type, osmium::Location>;

        // The location handler always depends on the index type
        using location_handler_type = osmium::handler::NodeLocationsForWays<index_type>;

        /**
         * 
         */
        FileData get_data(const std::string file_path, int territory_level, std::vector<int> bonus_levels)
        {   
            /* First pass */

            //
            osmium::io::File input_file{ file_path };

            // Initialize progress bar, enable it only if STDERR is a TTY.
            osmium::ProgressBar progress{input_file.size(), osmium::isatty(2)};

            // Configuration for the multipolygon assembler. Here the default settings
            // are used, but you could change multiple settings.
            osmium::area::Assembler::config_type assembler_config;
            
            // Set up a filter matching only boundaries that have the defined
            // territory level and bonus level(s). This implicitly also only
            // matches administrative boundaries, as these are the only relations
            // that can contain the admin_level tag.
            osmium::TagsFilter filter{ false };
            filter.add_rule(true, osmium::TagMatcher{
                "admin_level",
                boost::lexical_cast<std::string>(territory_level) 
            });
            for (auto& bonus_level : bonus_levels)
            {
                filter.add_rule(true, osmium::TagMatcher{
                    "admin_level",
                    boost::lexical_cast<std::string>(bonus_level) 
                });
            }

            // Initialize the MultipolygonManager. Its job is to collect all
            // relations and member ways needed for each area. It then calls an
            // instance of the osmium::area::Assembler class (with the given config)
            // to actually assemble one area. The filter parameter is optional, if
            // it is not set, all areas will be built.
            osmium::area::MultipolygonManager<osmium::area::Assembler> mp_manager{ assembler_config, filter };

            // Pass through file for the first time and feed relations to the
            // multipolygon manager
            osmium::relations::read_relations(input_file, mp_manager);

            /* Second pass */

            // The index map that maps node indices to their locations
            index_type index;
            
            // The handler that stores handles node indices in ways and relations.
            location_handler_type location_handler{ index };
            location_handler.ignore_errors();         
            
            //
            handler::AreaHandler area_handler {};

            //
            osmium::io::Reader reader{ input_file, osmium::io::read_meta::no };

            // Pass through file for the second time and process the objects
            FileData result;
            osmium::apply(reader, location_handler, mp_manager.handler(
                [&result, &area_handler](osmium::memory::Buffer&& buffer) {
                    osmium::apply(buffer, area_handler);
                    result.node_buffer = area_handler.node_buffer();
                    result.area_buffer = area_handler.area_buffer();
                    result.graph = area_handler.graph();
                })
            );

            // If there were multipolgyon relations in the input, but some of their
            // members are not in the input file (which often happens for extracts),
            // mark them as incomplete in the result data.
            std::vector<osmium::object_id_type> incomplete_relations_ids;
            mp_manager.for_each_incomplete_relation([&](const osmium::relations::RelationHandle& handle){
                result.incomplete_relations.push_back(handle->id());
            });

            return result;
        }

    }

}
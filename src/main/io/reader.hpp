#pragma once

#include <algorithm>
#include <array>
#include <fstream>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>

#include <osmium/visitor.hpp>
#include <osmium/io/any_input.hpp>
#include <osmium/io/any_output.hpp>
#include <osmium/io/file_format.hpp>
#include <osmium/index/map/flex_mem.hpp>
#include <osmium/tags/matcher.hpp>
#include <osmium/handler/node_locations_for_ways.hpp>

#include <nlohmann/json.hpp>

#include "handler/count_handler.hpp"
#include "handler/bounds_handler.hpp"
#include "handler/convert_handler.hpp"
#include "model/container.hpp"
#include "model/geometry/point.hpp"
#include "model/map/bonus.hpp"
#include "model/map/map.hpp"
#include "model/map/territory.hpp"
#include "model/memory/node.hpp"
#include "model/memory/way.hpp"
#include "model/memory/relation.hpp"
#include "model/memory/area.hpp"
#include "model/memory/buffer.hpp"
#include "model/type.hpp"

using namespace model;

namespace io
{

    namespace reader
    {

        /**
         * 
         */
        ConfigContainer read_config(const std::string& file_path)
        {
            // Read the json from the specified file path
            std::ifstream ifs { file_path };
            nlohmann::json json = nlohmann::json::parse(ifs);           
            // Parse json values as config and return the result
            return ConfigContainer{
                json.at("email"),
                json.at("api-token")
            };
        }

        /**
         * 
         */
        map::Map read_metadata(const std::string& file_path)
        {
            // Read the json from the specified file path
            std::ifstream ifs { file_path };
            nlohmann::json json = nlohmann::json::parse(ifs);    
            
            // Create the result map container
            map::Map map;

            // Parse the primitive json values
            map.name() = json.at("name");

            // Parse the territory information
            for (const auto& t : json.at("territories"))
            {   
                // Extract the territory meta information
                map::Territory territory{ t.at("id") };
                territory.name() = t.at("name");
                territory.center() = { t.at("center").at("x"), t.at("center").at("y") };
                for (const object_id_type& neighbor : t.at("neighbors"))
                {
                    territory.neighbors().push_back({ neighbor });
                }
                // Add territory to map container
                map.territories().push_back(territory);
            }

            // Parse the regular bonus information
            for (const auto& b : json.at("bonuses"))
            {   
                // Create regular bonus
                map::Bonus bonus{ b.at("id") };
                bonus.name() = b.at("name");
                bonus.color() = b.at("color");
                bonus.armies() = b.at("armies");
                for (const object_id_type& child : b.at("children"))
                {
                    bonus.children().push_back({ child });
                }
                // Add bonus to map container
                map.bonuses().push_back(bonus);
            }

            // Parse the super bonus information
            for (const auto& s : json.at("super_bonuses"))
            {
                // Create super bonus
                map::SuperBonus super_bonus{ s.at("id") };
                super_bonus.name() = s.at("name");
                super_bonus.color() = s.at("color");
                super_bonus.armies() = s.at("armies");
                for (const object_id_type& child : s.at("children"))
                {
                    super_bonus.children().push_back({ child });
                }             
                // Add super bonus to map container
                map.super_bonuses().push_back(super_bonus);    
            }
            
            // Return the resulting map container
            return map;
        }

        /**
         * 
         */
        InfoContainer read_fileinfo(const std::string& file_path)
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
            handler::TagValueCountHandler<level_type> level_counter{ "admin_level" };

            // Create the BoundsHandler that determines the bounds of the
            // input excerpt
            handler::BoundsHandler bounds_handler;

            // Apply the counters to the input file
            osmium::apply(reader, count_handler, level_counter, bounds_handler);

            // You do not have to close the Reader explicitly, but because the
            // destructor can't throw, you will not see any errors otherwise.
            reader.close();

            // Return results
            return {
                file_path,
                input_file.format(),
                input_file.compression(),
                input_file.buffer_size(),
                count_handler.node_count(),
                count_handler.way_count(),
                count_handler.relation_count(),
                bounds_handler.bounds(),
                level_counter.total(),
                level_counter.counts()
            };
        }

        // The type of index used. This must match the include file above
        using index_type = osmium::index::map::FlexMem<osmium::unsigned_object_id_type, osmium::Location>;

        // The location handler always depends on the index type
        using location_handler_type = osmium::handler::NodeLocationsForWays<index_type>;

        /**
         * 
         */
        DataContainer read_filedata(
            const std::string& file_path,
            level_type territory_level,
            std::vector<level_type> bonus_levels
        ) {   
            /* First pass */

            osmium::io::File input_file{ file_path };
    
            // Initialize the Converter. Its job is to collect all
            // relations and member ways needed for each relation.
            handler::ConvertHandler convert_handler{ territory_level, bonus_levels };

            // Pass through file for the first time and feed relations to the
            // multipolygon manager
            osmium::relations::read_relations(input_file, convert_handler);

            /* Second pass */

            // The index map that maps node indices to their locations
            index_type index;
            
            // The handler that stores handles node indices in ways and relations.
            location_handler_type location_handler{ index };
            location_handler.ignore_errors();         
            
            // Pass through file for the second time and process the objects
            osmium::io::Reader reader{ input_file, osmium::io::read_meta::no };            
            osmium::apply(reader, location_handler, convert_handler.handler());

            // Save results
            DataContainer data;
            data.nodes = convert_handler.nodes();
            data.ways = convert_handler.ways();
            data.relations = convert_handler.relations();

            // If there were multipolgyon relations in the input, but some of their
            // members are not in the input file (which often happens for extracts),
            // mark them as incomplete in the result data.
            std::vector<osmium::object_id_type> incomplete_relations_ids;
            convert_handler.for_each_incomplete_relation([&](const osmium::relations::RelationHandle& handle){
                data.incomplete_relations.push_back(handle->id());
            });

            return data;
        }

    }

}
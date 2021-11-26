#pragma once

#include <osmium/memory/buffer.hpp>
#include <osmium/relations/relations_manager.hpp>
#include <osmium/tags/tags_filter.hpp>
#include <osmium/io/any_input.hpp>

#include "handler/boundary_manager.hpp"
#include "io/reader/reader.hpp"
#include "model/types.hpp"

namespace io
{

    /**
     * A reader that retrieves the mapdata of an OSM file.
     */
    class BoundaryReader : public Reader<osmium::memory::Buffer>
    {
    protected:

        /**
         * The admin_level filter. Boundaries with an administrative level
         * contained in this set will be kept, while other boundaries will
         * be skipped.
         * 
         * OpenStreetMap defines 9 administrative levels from 2 to 11. Yet,
         * it is also possible to use the levels 0, 1 and 12, which are not
         * rendered by default, but need to be considered too.
         * 
         * For more information, refer to
         * https://wiki.openstreetmap.org/wiki/Key:admin_level
         * 
         */
        std::set<model::level_type> m_levels = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };

    public:

        /* Constructors */

        BoundaryReader(fs::path file_path) : Reader<osmium::memory::Buffer>(file_path) {}

        BoundaryReader(fs::path file_path, const std::set<model::level_type>& levels)
        : Reader<osmium::memory::Buffer>(file_path), m_levels(levels) {}

        /* Override Methods */

        osmium::memory::Buffer read() override
        {
            osmium::io::File file{m_path.string()};

            // Prepare the tag filter for the BoundaryManager with the
            // specified administrative levels
            osmium::TagsFilter filter{ false };
            for (const model::level_type& level : m_levels)
            {
                filter.add_rule(true, "admin_level", std::to_string(level));
            }

            // Instantiate the boundary filter, which will extract all
            // administrative boundary relation ids for the specified 
            // admin_levelsas as well as the associated way and node ids.
            handler::BoundaryManager manager{ filter };

            // First pass through the file: Read all relations and pass them to
            // the boundary manager. This will also filter out any relations that
            // do not match the filter.
            osmium::relations::read_relations(file, manager);

            // Second pass through the file: Extract the osmium objects through the
            // reader and pass them to the boundary manager, such that it can mark
            // the nodes, ways and relations
            osmium::io::Reader manager_reader{file};
            osmium::apply(manager_reader, manager.handler());
            manager.read();
            manager_reader.close();
            
            // Extract the matching ids from the manager and prepare the result buffer
            auto matching_ids = manager.matching_ids();
            osmium::memory::Buffer result{file.size(), osmium::memory::Buffer::auto_grow::yes};

            // If there were relations in the input with members that weren't
            // part of the input file (which often happens for extracts), write
            // the IDs of the incomplete relations to stderr.
            std::vector<osmium::object_id_type> incomplete_relations_ids;
            manager.for_each_incomplete_relation([&](const osmium::relations::RelationHandle& handle) {
                incomplete_relations_ids.push_back(handle->id());
            });
            if (!incomplete_relations_ids.empty())
            {
                std::cerr << "[Warning] Skipped missing members for "
                    << incomplete_relations_ids.size()
                    << " boundaries.\n";
            }

            // Third pass trough the file: Copy the marked object into the result buffer
            // through yet another reader. using the matching ids found by the boundary
            // manager.
            osmium::io::Reader copy_reader{file};
            while (osmium::memory::Buffer buffer = copy_reader.read())
            {
                // Copy the marked objects
                for (auto& object : buffer.select<osmium::OSMObject>())
                {
                    if (matching_ids(object.type()).get(object.positive_id()))
                    {
                        // Copy element to the buffer
                        result.add_item(object);
                        result.commit();
                    }
                }
            }
            copy_reader.close();

            // Close the reader and return the buffer.
            return std::move(result);
        }

    };

}
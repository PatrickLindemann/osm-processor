#pragma once

#include <osmium/osm/area.hpp>
#include <osmium/area/assembler.hpp>
#include <osmium/area/multipolygon_manager.hpp>
#include <osmium/handler/node_locations_for_ways.hpp>
#include <osmium/index/map/flex_mem.hpp>

#include "model/types.hpp"

namespace mapmaker
{

    class Assembler
    {
    protected:

        /* Types */

       /**
        * The type of index used. This must match the include file above
        */
        using index_type = osmium::index::map::FlexMem<osmium::unsigned_object_id_type, osmium::Location>;

        /**
         * The location handler always depends on the index type
         */
        using location_handler_type = osmium::handler::NodeLocationsForWays<index_type>;

        /* Members */

        /**
         * The admin_level filter. Boundary areas with an administrative level
         * contained in this set will be assembled, while other areas will
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

       /**
        * The split flag. If set to true, the assembled multipolygon areas,
        * which can contain multiple outer rings are split into polygon areas,
        * which contain exactly one outer ring (and n inner rings).
        */
        bool m_split;

    public:

        /* Constructors */

        Assembler() {}
        Assembler(const std::set<model::level_type>& levels, bool split = false) : m_levels(levels), m_split(split) {}
            
    protected:

        /* Helper Methods */

        void create_area_from_ring(osmium::memory::Buffer& buffer, const osmium::Area& area, const osmium::OuterRing& ring, osmium::object_id_type id)
        {
            // Create a new area from the specified outer ring by copying all
            // attributes and tags from the old area and inserting the inner
            // rings of the outer ring.
            osmium::builder::AreaBuilder area_builder{ buffer };

            // Copy the area attributes and tags
            area_builder.set_id(id)
                .set_version(area.version())
                .set_changeset(area.changeset())
                .set_timestamp(area.timestamp())
                .set_uid(area.uid())
                .set_user(area.user())
                .add_item(area.tags());

            // Copy the outer ring and the associated inner rings
            area_builder.add_item(ring);
            for (const osmium::InnerRing& inner : area.inner_rings(ring))
            {
                area_builder.add_item(inner);
            }

        }

    public:

        /* Methods */

        void run(osmium::memory::Buffer& buffer)
        {
            // Create the default configuration for the osmium assembler.
            osmium::area::Assembler::config_type config;

            // Prepare the tag filter for the multipolygon manager with the
            // specified administrative levels.
            osmium::TagsFilter filter{ false };
            for (const model::level_type& level : m_levels)
            {
                filter.add_rule(true, "admin_level", std::to_string(level));
            }

            // Initialize the MultipolygonManager. Its job is to collect all
            // relations and member ways needed for each area. It then calls an
            // instance of the osmium::area::Assembler class (with the given config)
            // to actually assemble one area.
            osmium::area::MultipolygonManager<osmium::area::Assembler> mp_manager{ config, filter };

            // First pass through the buffer: The manager will read all relation
            // and filter out any relations that do not match the filter.
            osmium::apply(buffer, mp_manager);
            mp_manager.prepare_for_lookup();

            // The index storing all node locations.
            index_type index;

            // The handler that stores all node locations in the index and adds them
            // to the ways.
            osmium::handler::NodeLocationsForWays<index_type> location_handler{ index };
            location_handler.ignore_errors();

            // Second pass through the buffer: Assemble the filtered boundary
            // relations into areas.
            osmium::apply(buffer, location_handler, mp_manager.handler());
            osmium::memory::Buffer area_buffer = mp_manager.read();

            // If there were boundary relations in the input with members that
            //  weren't part of the input file (which often happens for extracts),
            // write the IDs of the incomplete relations to stderr.
            std::vector<osmium::object_id_type> incomplete_relations_ids;
            mp_manager.for_each_incomplete_relation([&](const osmium::relations::RelationHandle& handle) {
                incomplete_relations_ids.push_back(handle->id());
                });
            if (!incomplete_relations_ids.empty()) {
                std::cerr << "[Warning] Skipped missing members for these boundaries: "
                    << util::join(incomplete_relations_ids)
                    << std::endl;
            }
      
            // Add the assembled areas from the area buffer to the input buffer
            std::size_t offset = 0;
            for (const osmium::Area& area : area_buffer.select<osmium::Area>())
            {
                if (m_split)
                {
                    for (const osmium::OuterRing& outer : area.outer_rings())
                    {
                        create_area_from_ring(buffer, area, outer, area.id() * (offset + 1));
                        buffer.commit();
                        ++offset;
                    }
                }
                else
                {
                    buffer.add_item(area);
                    buffer.commit();
                }
            }
        }

    };

}
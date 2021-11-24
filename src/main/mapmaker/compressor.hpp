#pragma once

#include <map>
#include <set>

#include <osmium/builder/osm_object_builder.hpp>
#include <osmium/handler/node_locations_for_ways.hpp>
#include <osmium/index/map/flex_mem.hpp>
#include <osmium/osm/types.hpp>

#include "handler/compression_handler.hpp"

namespace mapmaker
{

    class Compressor
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

        double m_tolerance;

    public:

        /* Constructors */

        /**
         * Run the compressor on the way and node buffers.
         * Nodes and ways that were removed by the compression will be
         * removed in the respective buffers.
         *
         * For more information on finding a good tolerance value, refer
         * to https://en.wikipedia.org/wiki/Ramer%E2%80%93Douglas%E2%80%93Peucker_algorithm
         *
         * @param tolerance The distance epsilon for the Douglas-Peucker-Algorithm.
         *
         * Time complexity: Log-Linear (Average-case), Quadratic (Worst-case)
         */
        Compressor(double tolerance) : m_tolerance(tolerance) {}
            
        /* Methods */

        void run(osmium::memory::Buffer& buffer)
        {
            // If the tolerance is less or equal to zero,
            // no compression will be applied.
            if (m_tolerance <= 0)
            {
                return;
            }

            // Calculate the degrees for each node in the input buffer. The
            // resulting map will indicate which nodes should be ignored during
            // the compression process in order to avoid the creation of holes
            // between boundaries.
            std::map<osmium::object_id_type, std::size_t> node_degrees{};
            for (const osmium::Way& way : buffer.select<osmium::Way>())
            {
                for (const osmium::NodeRef& nr : way.nodes())
                {
                    // Check if node entry exists in the count map
                    auto it = node_degrees.find(nr.ref());
                    if (it == node_degrees.end())
                    {
                        it = node_degrees.insert(it, { nr.ref(), 0 });
                    }
                    it->second += 1;
                }
            }

            // Ignore nodes that have more than two neighbors.
            std::set<osmium::object_id_type> ignored_nodes;
            for (const auto& [id, degree] : node_degrees)
            {
                if (degree > 2)
                {
                    ignored_nodes.insert(id);
                }
            }

            // The index storing all node locations.
            index_type index;

            // The handler that stores all node locations in the index and adds them
            // to the ways.
            osmium::handler::NodeLocationsForWays<index_type> location_handler{ index };
            location_handler.ignore_errors();

            // Compress the ways in the buffer using the Douglas-Peucker
            // algorithm and retrieve the set of removed node ids.
            handler::CompressionHandler compression_handler{ m_tolerance, ignored_nodes };
            osmium::apply(buffer, location_handler, compression_handler);
            std::set<osmium::object_id_type> removed_nodes = compression_handler.removed_nodes();

            // Create a new buffer by copying the objects from the old buffer
            // while ignoring nodes that were marked as removed by the
            // compression handler.
            osmium::memory::Buffer result{ 1024, osmium::memory::Buffer::auto_grow::yes };
            for (const auto& object : buffer.select<osmium::OSMObject>())
            {
                switch (object.type())
                {
                case osmium::item_type::node:
                    // Copy the node if it was not marked as removed
                    if (!removed_nodes.count(object.id()))
                    {
                        result.add_item(object);
                        result.commit();
                    }
                    break;
                case osmium::item_type::way:
                    {
                        // Copy the way and rebuild the node reference list
                        // while ignoring the removed node references.
                        osmium::builder::WayBuilder way_builder{ result };

                        // Copy the way attributes and tags
                        way_builder.set_id(object.id())
                            .set_version(object.version())
                            .set_changeset(object.changeset())
                            .set_timestamp(object.timestamp())
                            .set_uid(object.uid())
                            .set_user(object.user())
                            .add_item(object.tags());

                        // Copy the node references and filter the compressed nodes
                        {
                            const osmium::Way& way = static_cast<const osmium::Way&>(object);
                            osmium::builder::WayNodeListBuilder way_nodes_builder{ way_builder };
                            for (const osmium::NodeRef& nr : way.nodes())
                            {
                                if (!removed_nodes.count(nr.ref()))
                                {
                                    way_nodes_builder.add_node_ref(nr.ref());
                                }
                            }
                        }
                    }
                    result.commit();
                    break;
                default:
                    result.add_item(object);
                    result.commit();
                }
            }

            // Swap the old buffer with the resulting compressed buffer
            std::swap(buffer, result);
        }

    };

}
#ifndef CONVERTER_HANDLER_HPP
#define CONVERTER_HANDLER_HPP

#include <cmath>
#include <string>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>

#include <osmium/osm/area.hpp>
#include <osmium/osm/node_ref.hpp>
#include <osmium/osm/node_ref_list.hpp>
#include <osmium/osm/types.hpp>
#include <osmium/handler.hpp>

#include "model/memory/area.hpp"
#include "model/memory/node.hpp"
#include "model/memory/types.hpp"
#include "model/memory/buffer.hpp"
#include "model/graph/undirected_graph.hpp"

using namespace model;

namespace handler
{

    /* Definitions */

    using NodeBuffer  = memory::Buffer<memory::Node>;
    using AreaBuffer  = memory::Buffer<memory::Area>;
    using Graph       = graph::UndirectedGraph;

    /**
     * 
     */
    class AreaHandler : public osmium::handler::Handler
    {
    public:

        /* Types */

        using id_type = memory::object_id_type;
        using id_map  = std::unordered_map<osmium::object_id_type, id_type>;

    protected:

        /* Types */

        using ring_type = memory::Area::ring_type;

        /* Members */

        /**
         * 
         */
        id_map m_node_ids;

        /**
         * 
         */
        id_type m_area_id = 0;

        /**
         * 
         */
        id_type m_ring_id = 0;

        /**
         * 
         */
        NodeBuffer m_node_buffer;

        /**
         * 
         */
        AreaBuffer m_area_buffer;

        /**
         * 
         */
        Graph m_graph;

        /* Functions */

        /**
         * Maps an osmium object id to an id in the continous, natural interval [0, N].
         * This bijective mapping ensures that all internal node ids are consecutive,
         * which is not guaranteed for osmium object ids.
         * 
         * @param id The osmium object id
         * @returns The internal object id in the interval [0, N]
         */
        const memory::object_id_type map(id_map& id_map, const osmium::object_id_type id)
        {
            // Check if id was mapped already
            if (id_map.find(id) == id_map.end())
            {
                id_map[id] = id_map.size();
            }
            return id_map[id];
        } 

        /**
         * 
         */
        ring_type create_ring(ring_type::id_type id, const osmium::NodeRefList& nodes)
        {
            ring_type::container_type ref_list;
            id_type last_id = -1;
            // Add nodes from the NodeRefList to the node & area buffer and
            // the graph
            for (const osmium::NodeRef& node : nodes)
            {                    
                // Map the osmium id to the internal id interval [0, N]
                id_type id = map(m_node_ids, node.ref());
                // Add node to the node buffer if it wasn't inserted already
                if (!m_node_buffer.contains(id))
                {
                    // Insert node into buffer
                    m_node_buffer.append(memory::Node{ id, node.lon(), node.lat() });
                }
                // Add node reference to the current ring
                ref_list.push_back(id);
                // Create an edge from the last to the current vertex in the graph
                if (last_id > 0)
                {
                    m_graph.insert_edge(std::make_pair(last_id, id));
                }
                last_id = id;
            }
            return ring_type{ id, ref_list };
        }

    public:

        /* Constructors */

        AreaHandler() {};

        /* Accessors */

        NodeBuffer& node_buffer()
        {
            return m_node_buffer;
        }

        AreaBuffer& area_buffer()
        {
            return m_area_buffer;
        }

        Graph& graph()
        {
            return m_graph;
        }

        /* Osmium functions */

        void area(const osmium::Area& osm_area)
        {    
            // Retrieve tag values
            const char * name = osm_area.get_value_by_key("name", "");
            const char * boundary = osm_area.get_value_by_key("boundary", "");
            const char * admin_level = osm_area.get_value_by_key("admin_level", "0");

            // Create new internal area
            memory::Area area{
                m_area_id++,
                name,
                boundary,
                std::stoi(admin_level)
            };

            // Create the rings and add them to the area
            // This implicitly adds the contained nodes to the node buffer
            // and the vertices and edges to the graph
            for (const auto& osm_outer_ring : osm_area.outer_rings())
            {
                // Add outer ring to the area
                ring_type outer_ring = create_ring(m_ring_id++, osm_outer_ring);
                area.add_outer(outer_ring);
                // Add inner rings of this outer ring to the area
                for (const auto& osm_inner_ring : osm_area.inner_rings(osm_outer_ring))
                {
                    ring_type inner_ring = create_ring(m_ring_id++, osm_inner_ring);
                    area.add_inner(outer_ring, inner_ring);
                }
            }

            // Add the new area to the buffer
            m_area_buffer.append(area);
        }

    };

}

#endif
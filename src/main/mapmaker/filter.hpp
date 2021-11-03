#pragma once

#include <utility>
#include <vector>

#include "functions/area.hpp"
#include "model/geometry/rectangle.hpp"
#include "model/graph/edge.hpp"
#include "model/graph/undirected_graph.hpp"
#include "model/memory/area.hpp"
#include "model/memory/buffer.hpp"
#include "model/memory/entity.hpp"
#include "model/memory/node.hpp"
#include "model/memory/relation.hpp"
#include "model/memory/way.hpp"
#include "model/type.hpp"

using namespace model;

namespace mapmaker
{

    namespace filter
    {
        
        /**
         * A filter that removes areas based on their surface area.
         */
        class AreaFilter
        {
        public:

            /* Types */

            using component_map_type = std::unordered_map<component_type, std::vector<object_id_type>>;
            using id_map_type  = std::unordered_map<object_id_type, object_id_type>;

        protected:

            /* Members */

            memory::Buffer<memory::Area>& m_area_buffer;
            memory::Buffer<memory::Relation>& m_relation_buffer;
            graph::UndirectedGraph& m_neighbors;
            component_map_type& m_components;
            const memory::Buffer<memory::Node>& m_node_buffer;
            const memory::Buffer<memory::Way>& m_way_buffer;
            
        public:

            /* Constructors */

            AreaFilter(
                memory::Buffer<memory::Area>& areas,
                memory::Buffer<memory::Relation>& relations,
                graph::UndirectedGraph& neighbors,
                component_map_type& components,
                const memory::Buffer<memory::Node>& nodes,
                const memory::Buffer<memory::Way>& ways
            ) : m_area_buffer(areas), m_relation_buffer(relations), m_neighbors(neighbors),
                m_components(components), m_node_buffer(nodes), m_way_buffer(ways) {};
                
            /* Methods */

            /**
             * Apply the filter on the specified area buffer.
             * Areas that have a smaller surface area relative to the
             * total surface area than the specified threshold will be
             * removed.
             * 
             * @param threshold The relative surface area threshold
             * 
             * Time complexity: Linear
             */
            void filter_areas(double threshold)
            {

                std::vector<bool> removed_areas(m_area_buffer.size());
                std::vector<bool> removed_ways(m_way_buffer.size());

                // Pre-calculate the surface areas for areas with the
                // specified level and compute the total surface area
                double total_surface_area = 0.0;
                std::unordered_map<object_id_type, double> surface_areas;
                for (const memory::Area& area : m_area_buffer)
                {
                    // Convert the outer ring of the area
                    // Note: Areas of the specified level must have
                    // exactly one outer ring.
                    geometry::Ring<double> outer;
                    for (const memory::NodeRef& nr : area.outer_rings().at(0))
                    {
                        const memory::Node& node = m_node_buffer.at(nr);
                        outer.push_back(node.point());
                    }
                    double surface_area = functions::area(outer);
                    surface_areas[area.id()] = surface_area;
                    total_surface_area += surface_area;
                }

                // Filter components by their surface area
                for (auto it = m_components.cbegin(); it != m_components.cend();)
                {
                    // Get the component surface area
                    double component_surface_area = 0.0;
                    for (const object_id_type ar : it->second)
                    {
                        component_surface_area += surface_areas.at(ar);
                    }
                    // Check if the relative component area to total area
                    // ratio is lower than the specified threshold
                    double relative_surface_area = component_surface_area / total_surface_area;
                    if (relative_surface_area < threshold)
                    {
                        // Mark all areas and their way references in that component
                        // for removal
                        for (const object_id_type ar : it->second)
                        {
                            const memory::Area& area = m_area_buffer.at(ar);
                            for (const memory::WayRef& wr : area.ways())
                            {
                                removed_ways.at(wr.ref()) = true;
                            }
                            removed_areas.at(ar) = true;
                        }
                        // Remove component from the component map
                        m_components.erase(it++);
                    }
                    else
                    {
                        ++it;
                    }
                }

                // Removed marked ways from the relation buffer
                for (memory::Relation& relation : m_relation_buffer)
                {
                    for (auto it = relation.members().begin(); it != relation.members().end(); it++)
                    {
                        if (removed_ways.at(it->ref()))
                        {
                            relation.members().erase(it--);
                        }
                    }
                }

                // Remove marked areas from the buffer and perform
                // an area reindex (on the buffer and graph)
                id_map_type a_ids;
                memory::Buffer<memory::Area> new_area_buffer;
                graph::UndirectedGraph new_neighbors;
                for (const memory::Area& area : m_area_buffer)
                {
                    if (!removed_areas.at(area.id()))
                    {
                        object_id_type mapped_id = a_ids.size();
                        a_ids[area.id()] = mapped_id;
                        // Create the area copy with the mapped id
                        memory::Area new_area{
                            mapped_id,
                            area.name(),
                            area.level(),
                            area.original_id()
                        };
                        // Copy the area rings
                        for (const memory::Ring& outer : area.outer_rings())
                        {
                            new_area.add_outer(outer);
                            for (const memory::Ring& inner : area.inner_rings(outer))
                            {
                                new_area.add_inner(outer, inner);
                            }
                        }
                        // Copy the area way references that were not marked as removed
                        for (const memory::WayRef& way : area.ways())
                        {
                            if (!removed_ways.at(way.ref()))
                            {
                                new_area.add_way(way);
                            }
                        }
                        new_area_buffer.push_back(new_area);
                        new_neighbors.insert_vertex(mapped_id);
                    }
                }
                std::swap(m_area_buffer, new_area_buffer);

                // Add the new neighbor edges
                for (const graph::edge_type& edge : m_neighbors.edges())
                {
                    if (!removed_areas.at(edge.first) && !removed_areas.at(edge.second))
                    {
                        const graph::vertex_type& v1 = a_ids.at(edge.first);
                        const graph::vertex_type& v2 = a_ids.at(edge.second);
                        new_neighbors.vertices().insert(v1);
                        new_neighbors.edges().insert({ v1, v2 });
                    }
                }
                std::swap(m_neighbors, new_neighbors);
            }

        };

    }

}
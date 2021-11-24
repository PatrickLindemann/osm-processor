#pragma once

#include <numeric>

#include <osmium/osm/node.hpp>
#include <osmium/osm/area.hpp>

#include "model/graph/undirected_graph.hpp"

#include "handler/calculation_handler.hpp"
#include "handler/filter_handler.hpp"

using namespace model;

namespace mapmaker
{
        
    /**
     * A filter that removes areas based on their surface area.
     */
    class AreaFilter
    {
    protected:

        /* Members */

        double m_tolerance;
            
    public:

        /* Constructors */

        AreaFilter(double tolerance) : m_tolerance(tolerance) {}
                
        /* Methods */

        /**
         * Apply the filter on the specified area buffer.
         * Areas that have a smaller surface area relative to the
         * total surface area than the specified threshold will be
         * removed.
         *
         *
         * Time complexity: Linear
         */
        void run(
            osmium::memory::Buffer& buffer,
            graph::UndirectedGraph& neighbors,
            std::vector<std::set<osmium::object_id_type>>& components
        ){
            // Calculate the surface areas of each area in the buffer.
            handler::SurfaceAreaHandler surface_handler{};
            osmium::apply(buffer, surface_handler);
            std::map<osmium::object_id_type, double> area_surfaces = surface_handler.surfaces();
            double total_surface = surface_handler.total();

            // Filter components by checking if their relative surface area is
            // less than the specified threshold
            std::set<osmium::object_id_type> removed_areas{};
            for (std::size_t i = 0; i < components.size();)
            {
                // Calculate the component surface area
                double component_surface = 0;
                for (const osmium::object_id_type& id : components.at(i))
                {
                    component_surface += area_surfaces.at(id);
                }

                double relative_surface = component_surface / total_surface;
                if (relative_surface < m_tolerance)
                {
                    // Mark all areas in the component for removal
                    for (const osmium::object_id_type& id : components.at(i))
                    {
                        removed_areas.insert(id);
                    }
                    components.erase(components.begin() + i);
                }
                else
                {
                    i++;
                }
            }

            // Check if any areas were marked for removal before continuing
            if (removed_areas.empty())
            {
                return;
            }
            
            // Retrieve the node references for the removed areas
            handler::AreaNodeFilterHandler node_handler{ removed_areas };
            osmium::apply(buffer, node_handler);
            std::set<osmium::object_id_type> removed_nodes = node_handler.references();

            // Retrieve the way references for the removed areas
            handler::NodeWayFilterHandler way_handler{ removed_nodes };
            osmium::apply(buffer, way_handler);
            std::set<osmium::object_id_type> removed_ways = way_handler.references();

            // Remove the marked areas and their associated nodes and ways from
            // the buffer
            osmium::memory::Buffer result{ 1024, osmium::memory::Buffer::auto_grow::yes };
            for (const auto& object : buffer.select<osmium::OSMObject>())
            {
                switch (object.type())
                {
                case osmium::item_type::node:
                    if (!removed_nodes.count(object.id()))
                    {
                        result.add_item(object);
                        result.commit();
                    }
                    break;
                case osmium::item_type::way:
                    if (!removed_ways.count(object.id()))
                    {
                        if (!removed_areas.count(osmium::object_id_to_area_id(object.id(), object.type())))
                        {
                            result.add_item(object);
                            result.commit();
                        }
                    }
                    break;
                case osmium::item_type::relation:
                    if (!removed_areas.count(osmium::object_id_to_area_id(object.id(), object.type())))
                    {
                        {
                            // Rebuild relation without the removed way members
                            osmium::builder::RelationBuilder relation_builder{ result };

                            // Copy the way attributes and tags
                            relation_builder.set_id(object.id())
                                .set_version(object.version())
                                .set_changeset(object.changeset())
                                .set_timestamp(object.timestamp())
                                .set_uid(object.uid())
                                .set_user(object.user())
                                .add_item(object.tags());

                            {
                                // Copy the relation references and filter the removed ways
                                const osmium::Relation& relation = static_cast<const osmium::Relation&>(object);
                                osmium::builder::RelationMemberListBuilder members_builder{ relation_builder };
                                for (const osmium::RelationMember& member : relation.members())
                                {
                                    if (member.type() != osmium::item_type::way || !removed_ways.count(member.ref()))
                                    {
                                        members_builder.add_member(member.type(), member.ref(), member.role());
                                    }
                                }
                            }
                        }
                        result.commit();
                    }
                    break;
                case osmium::item_type::area:
                    if (!removed_areas.count(object.id()))
                    {
                        result.add_item(object);
                        result.commit();
                    }
                    break;
                }
            }
            std::swap(buffer, result);

            // Remove the marked areas and their neighbors from the neighbor
            // graph by creating a filtered copy.
            graph::UndirectedGraph result_neighbors{};
            for (const graph::vertex_type& vertex : neighbors.vertices())
            {
                if (!removed_areas.count(vertex))
                {
                    result_neighbors.insert_vertex(vertex);
                }
            }
            for (const graph::edge_type& edge : neighbors.edges())
            {
                if (!removed_areas.count(edge.first) && !removed_areas.count(edge.second))
                {
                    result_neighbors.insert_edge(edge);
                }
            }
            std::swap(neighbors, result_neighbors);

        }

    };

}
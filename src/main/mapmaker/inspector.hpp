#pragma once

#include <bits/stdint-intn.h>
#include <cassert>
#include <stack>
#include <unordered_map>
#include <vector>

#include "model/geometry/rectangle.hpp"
#include "model/graph/edge.hpp"
#include "model/graph/undirected_graph.hpp"
#include "model/graph/vertex.hpp"
#include "model/memory/area.hpp"
#include "model/memory/buffer.hpp"
#include "model/memory/entity.hpp"
#include "model/memory/way.hpp"

using namespace model;

namespace mapmaker
{

    namespace inspector
    {
        
        using namespace model::memory;
        using namespace model::graph;

        /**
         * A class for finding neighborship relations and connected
         * components on a set of areas.
         */
        class NeighborInspector
        {

            /* Types */

            using component_map_type = std::unordered_map<component_type, std::vector<AreaRef>>;

            /* Members */

            const Buffer<Area>& m_area_buffer;

        public:

            /* Constructors */

            NeighborInspector(const Buffer<Area>& areas) : m_area_buffer(areas) {};

        protected:

            /* Helper methods */

            /**
             * Create the neighborship graph for the specified
             * areas by checking which areas reference the same
             * ways. If they do so, they are considered as
             * neighbors.
             * 
             * @returns The neighbor graph
             * 
             * Time complexity: Linear
             */
            UndirectedGraph calculate_neighbors()
            {
                UndirectedGraph neighbors;
                // Prepare the way reference map that holds a list of areas
                // that reference a way with a specified id.
                std::unordered_map<object_id_type, std::vector<object_id_type>> way_area_map;
                // Fill the way map
                for (const Area& area : m_area_buffer)
                {
                    for (const WayRef& way : area.ways())
                    {
                        auto it = way_area_map.find(way.ref());
                        if (it == way_area_map.end())
                        {
                            it = way_area_map.insert(it, { way.ref(), {} });
                        }
                        it->second.push_back(area.id());
                    }
                }
                // Create edges in the graph for each two areas that share the
                // same way
                for (const auto& [way_ref, area_refs] : way_area_map)
                {
                    for (size_t i = 0; i < area_refs.size() - 1; i++)
                    {
                        for (size_t j = i + 1; j < area_refs.size(); j++)
                        {
                            neighbors.insert_edge({ area_refs.at(i), area_refs.at(j) });
                        }   
                    }
                }
                return std::move(neighbors);
            }

            /**
             * Perform a Depth-First-Search on a neighbor graph
             * and mark all vertices that are connected to each
             * other as part of the same component.
             * 
             * @param neighbors The neighbor graph
             * @returns         The map of connected components
             * 
             * Time complexity: Linear
             */
            inline component_map_type calculate_components(const UndirectedGraph& neighbors)
            {
                component_map_type components;
                if (neighbors.vertex_count() == 0)
                {
                    return std::move(components);
                }
                // The vertices that have been processed already
                std::vector<bool> processed(neighbors.vertex_count());
                //
                unsigned long current_component = 0;
                for (vertex_type vertex = 0; vertex < neighbors.vertex_count(); vertex++)
                {
                    // Check if the current vertex was visited already
                    if (!processed[vertex])
                    {
                        // Create new component with the current component index
                        components[current_component] = {};
                        // Perform dfs with the current vertex as start
                        std::stack<vertex_type> stack;
                        stack.push(vertex);
                        processed[vertex] = true;
                        while (!stack.empty())
                        {
                            vertex_type v = stack.top();
                            stack.pop();
                            // Mark the current vertex as part of the component
                            components.at(current_component).push_back({ v });
                            // Mark all adjacents of this vertex if they haven't
                            // been visited already
                            for (const auto& adjacent : neighbors.adjacents(v))
                            {
                                if (!processed[adjacent])
                                {
                                    stack.push(adjacent);
                                    processed[adjacent] = true;
                                }
                            }
                        }
                        ++current_component;
                    }
                }
                return std::move(components);
            }
        
        public:

            /* Methods */

            /**
             * Calculate the neighbor relations and components for the
             * specified areas.
             * 
             * @returns The pair <NeighborGraph, ComponentMap>
             * 
             * Time complexity: Linear
             */
            std::pair<UndirectedGraph, component_map_type> get_relations()
            {
                // Create the neighbor graph for the areas.
                // Vertices represent the areas, edges represent a
                // neighborship relation between them.
                // Note: This graph is undirected, as the neighborship
                // relation is symmetric.
                UndirectedGraph neighbors = calculate_neighbors();
                // Create the component map for the areas.
                // This map specifies which areas are inter-connected
                // between each other.
                component_map_type components = calculate_components(neighbors);
                // Return the results
                return std::make_pair(neighbors, components);
            }

        };

    }

}
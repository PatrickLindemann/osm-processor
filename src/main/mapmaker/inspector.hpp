#pragma once

#include <cassert>
#include <stack>
#include <vector>
#include <unordered_map>

#include "model/geometry/rectangle.hpp"
#include "model/memory/types.hpp"
#include "model/memory/way.hpp"
#include "model/memory/area.hpp"
#include "model/memory/buffer.hpp"
#include "model/graph/graph.hpp"
#include "model/graph/undirected_graph.hpp"
#include "functions/envelope.hpp"
#include "functions/intersect.hpp"

using namespace model;

namespace mapmaker
{

    namespace inspector
    {

        template <typename T>
        class Inspector
        {
        public:

            using id_type = memory::object_id_type;
            using level_type = unsigned short;
            using component_type = unsigned long;

            template <typename K>
            using map_type = std::unordered_map<K, std::vector<id_type>>;

        protected:

            memory::Buffer<memory::Node<T>>& m_nodes;
            memory::Buffer<memory::Way<T>>& m_ways;
            memory::Buffer<memory::Area<T>>& m_areas;

            /**
             * 
             */
            graph::UndirectedGraph m_neighbors;
            
            /**
             * 
             */
            map_type<component_type> m_components;

            /**
             * 
             */
            map_type<id_type> m_subareas;

        public:

            Inspector(
                memory::Buffer<memory::Node<T>>& nodes,
                memory::Buffer<memory::Way<T>>& ways,
                memory::Buffer<memory::Area<T>>& areas
            ) : m_nodes(nodes), m_ways(ways), m_areas(areas) {};

            graph::UndirectedGraph neighbors()
            {
                return m_neighbors;
            }

            map_type<component_type> components()
            {
                return m_components;
            }

            map_type<id_type> subareas()
            {
                return m_subareas;
            }

        protected:

            /**
             * 
             * Time complexity: 
             */
            void calculate_neighbors()
            {
                // Prepare the way reference map that holds a list of areas
                // that reference a way with a specified id.
                std::vector<std::vector<id_type>> way_references(m_ways.size());
                // Fill the way map
                for (const memory::Area<T>& area : m_areas)
                {
                    for (const id_type& way_ref : area.way_references())
                    {
                        way_references.at(way_ref).push_back(area.id());
                    }
                }
                // Create edges in the graph for each two areas that share the
                // same way
                for (const auto& area_list : way_references)
                {
                    for (size_t i = 0; i < area_list.size(); i++)
                    {
                        for (size_t j = i + 1; j < area_list.size(); j++)
                        {
                            m_neighbors.insert_edge({ area_list.at(i), area_list.at(j) });
                        }   
                    }
                }
            }

            /**
             * Performs a Depth-First-Search on a vertex for a specified graph and
             * marks all vertices that are connected to this vertex as part of the
             * same specified component.
             * @param result The result list of component ids, where the index represents a vertex.
             * @param count The resulting count for the current component
             * @param vertex The graph vertex
             * @param component The current component id.
             * @param graph The graph.
             *
             * Time complexity: // TODO
             */
            inline void calculate_components()
            {
                if (m_neighbors.vertex_count() == 0)
                {
                    return;
                }
                // The vertices that have been processed already
                std::vector<bool> processed(m_neighbors.vertex_count());
                //
                unsigned long current_component = 0;
                for (graph::Graph::vertex_type vertex = 0; vertex < m_neighbors.vertex_count(); vertex++)
                {
                    // Check if the current vertex was visited already
                    if (!processed[vertex])
                    {
                        // Create new component with the current component index
                        m_components[current_component] = {};
                        // Perform dfs with the current vertex as start
                        std::stack<graph::Graph::vertex_type> stack;
                        stack.push(vertex);
                        processed[vertex] = true;
                        while (!stack.empty())
                        {
                            graph::Graph::vertex_type v = stack.top();
                            stack.pop();
                            // Mark the current vertex as part of the component
                            m_components.at(current_component).push_back(v);
                            // Mark all adjacents of this vertex if they haven't
                            // been visited already
                            for (const auto& adjacent : m_neighbors.adjacents(v))
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
            }

            /**
             * 
             */
            inline void calculate_subareas()
            {
                // Create the hirarchy, which groups all areas to their level
                std::set<level_type> level_set;
                map_type<level_type> hirarchy;
                for (const memory::Area<T>& area : m_areas)
                {
                    auto it = hirarchy.find(area.level());
                    if (it == hirarchy.end())
                    {
                        it = hirarchy.insert(it, { area.level(), {} });
                    }
                    it->second.push_back(area.id());
                    level_set.insert(area.level());
                }
                if (level_set.size() == 1)
                {
                    // Only areas with one type of level were extracted,
                    // no subarea relations are possible
                    return;
                }

                // Calculate the envelopes for all areas
                std::vector<geometry::Rectangle<T>> envelopes;
                for (const memory::Area<T>& area : m_areas)
                {
                    envelopes.push_back(functions::envelope(area, m_nodes));
                }

                // Iterate the level pairs
                std::vector<level_type> levels(level_set.size());
                std::copy(level_set.begin(), level_set.end(), levels.begin());
                for (size_t i = 0, j = 1; j < levels.size(); i++, j++)
                {
                    level_type high_level = levels.at(i);
                    level_type low_level = levels.at(j);

                    // Retrieve the area references for the current two levels
                    std::vector<id_type>& high_areas = hirarchy.at(high_level);
                    std::vector<id_type>& low_areas = hirarchy.at(low_level);

                    // Prepare the subarea map for the high areas
                    for (const id_type& high_ref : high_areas)
                    {
                        m_subareas[high_ref] = {};
                    }      

                    // Determine the subarea relations
                    for (const id_type& low_ref : low_areas)
                    {
                        // Find bonus candidates by checking if the lower
                        // area bounding box is contained in the higher area
                        // bounding box
                        std::vector<id_type> candidates;
                        for (const id_type& high_ref : high_areas)
                        {
                            if (functions::rectangle_in_rectangle(
                                envelopes.at(low_ref),
                                envelopes.at(high_ref)
                            )) {
                               candidates.push_back(high_ref);
                            }
                        }
                        // Find the bonus by polygon-in-polygon tests
                        for (const id_type& candidate : candidates)
                        {
                            //if (area_in_area(low_ref, candidate))
                            //{
                                m_subareas.at(candidate).push_back(low_ref);
                                break;
                            //}
                        }
                        // No bonus was found for the current area
                    }

                    // Verify that every higher area has at least one subarea
                    for (const id_type& high_ref : high_areas)
                    {
                        assert(m_subareas.at(high_ref).size() > 0);
                    }

                }

            }
        
        public:

            void run()
            {
                calculate_neighbors();
                calculate_components();
                calculate_subareas();
            }

        };

    }

}
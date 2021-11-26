#pragma once

#include <map>
#include <set>

#include "model/graph/undirected_graph.hpp"

#include "functions/intersect.hpp"

#include "util/insert.hpp"

namespace mapmaker
{

    /**
     * A class for finding neighborship relations on a collection of areas.
     */
    class NeighborInspector
    {

        /* Members */

        model::level_type m_level;

    public:

        /* Constructors */

        NeighborInspector(model::level_type level) : m_level(level) {};

        /* Methods */

        /**
         * Create the neighbor graph for a osmium buffer of areas by checking
         * which areas contain the same node references. If they do so, they are
         * considered to be neighbors. As the neighbor relation is symmetric,
         * the graph is chosen as undirected.
         * 
         * @returns The neighbor graph, where vertices represent the areas and
         *          edges represent a neighborship between to areas
         * 
         * Time complexity: Linear
         */
        model::graph::UndirectedGraph run(const osmium::memory::Buffer& buffer)
        {
            graph::UndirectedGraph neighbors;
            
            std::map<osmium::object_id_type, std::set<osmium::object_id_type>> references{};
            for (const osmium::Area& area : buffer.select<osmium::Area>())
            {
                // Create a vertex for the area in the neighbor graph
                neighbors.insert_vertex(area.id());

                // Collect the node references for this area
                for (const osmium::OuterRing& outer : area.outer_rings())
                {
                    for (const osmium::NodeRef& nr : outer)
                    {
                        util::insert(references, nr.ref(), area.id());
                    }
                    for (const osmium::InnerRing& inner : area.inner_rings(outer))
                    {
                        for (const osmium::NodeRef& nr : inner)
                        {
                            util::insert(references, nr.ref(), area.id());
                        }
                    }
                }
            }

            // Create edges in the graph for each two areas that share a common
            // node.
            for (const auto& [node, areas] : references)
            {
                for (auto it1 = areas.begin(); it1 != areas.end(); it1++)
                {
                    for (auto it2 = std::next(it1, 1); it2 != areas.end(); it2++)
                    {
                        neighbors.insert_edge(std::make_pair(*it1, *it2));
                    }
                }
            }

            return neighbors;
        }

    };

    class ComponentInspector
    {
    public:

        /* Constructors */

        ComponentInspector() {}

        /* Methods */

        /**
         * Retrieve the connected components of a neighborsship graph, where 
         * each vertex has a path to any other vertex in the same component.
         * Components are calculated by performing a depth-first search on the
         * specified neighbor graph.
         *
         * @param neighbors The neighbor graph
         * @returns         The list of connected components, where the index
         *                  represents the component and the set the areas that
         *                  are contained in it
         *
         * Time complexity: Linear
         */
        std::vector<std::set<osmium::object_id_type>> run(const model::graph::UndirectedGraph& neighbors)
        {
            if (neighbors.vertex_count() == 0)
            {
                return {};
            }

            // Prepare the component map that stores the component for each
            // vertex in the neighbor graph.
            std::map<osmium::object_id_type, std::size_t> components;

            // Calculate the connected components.
            std::size_t c = 0;
            for (const graph::vertex_type& vertex : neighbors.vertices())
            {
                // Check if the current vertex was visited already
                if (!components.count(vertex))
                {
                    // Perform depth-first-search with the current vertex as
                    // starting point
                    std::stack<graph::vertex_type> stack;
                    stack.push(vertex);
                    while (!stack.empty())
                    {
                        graph::vertex_type v = stack.top();
                        stack.pop();
                        // Mark the current vertex as part of the current
                        // component and add its unvisited adjacents to
                        // the stack for the next iterations.
                        components[v] = c;
                        for (const auto& adjacent : neighbors.adjacents(v))
                        {
                            if (!components.count(adjacent))
                            {
                                stack.push(adjacent);
                            }
                        }
                    }
                    // All vertices that were reachable from the chosen start
                    // vertex were vistied, therefore the connected component
                    // is complete.
                    ++c;
                }
            }

            // Reverse the component map, such that the component becomes the
            // index and the areas become the values.
            std::vector<std::set<osmium::object_id_type>> result(c, std::set<osmium::object_id_type>{});
            for (const auto& [area, component] : components)
            {
                result.at(component).insert(area);
            }

            return result;
        }

    };

    template <typename T>
    class HierarchyInspector
    {
    public:

        /* Types */

        using hierarchy_t = std::map<object_id_type, std::set<object_id_type>>;

    protected:

        /* Members */

    public:

        /* Constructors */

        HierarchyInspector() {}

    protected:

        /* Helper Methods */

        object_id_type group(
            const std::map<object_id_type, Boundary<T>>& boundaries,
            object_id_type id,
            std::set<object_id_type> candidates
        ) {
            // Retrieve the child boundary
            const Boundary<T>& child = boundaries.at(id);
            for (const object_id_type& c : candidates)
            {
                // Retrieve the potential parent boundary
                const Boundary<T>& candidate = boundaries.at(c);
                // Compare bounding boxes first
                if (!functions::rectangle_in_rectangle(child.bounds, candidate.bounds))
                {
                    continue;
                }
                // Compare the actual geometries
                for (const geometry::Polygon<T>& p_child : child.geometry.polygons())
                {
                    for (const geometry::Polygon<T>& p_candidate : candidate.geometry.polygons())
                    {
                        if (functions::polygon_in_polygon(p_child, p_candidate))
                        {
                            // Parent found
                            return c;
                        }
                    }
                }
            }
            // No parent found
            return -1;
        }

    public:

        /* Methods */

        hierarchy_t run(const std::map<object_id_type, Boundary<T>>& boundaries)
        {
            std::map<level_type, std::set<object_id_type>> level_map;
            for (const auto& [id, boundary] : boundaries)
            {
                util::insert(level_map, boundary.level, boundary.id);
            }
            
            hierarchy_t hierarchy;
            if (level_map.size() < 2)
            {
                return hierarchy;
            }

            // Group each two levels
            for (auto it_h = level_map.rbegin(); it_h != level_map.rend(); it_h++)
            {
                auto it_l = std::next(it_h, 1);
                if (it_l == level_map.rend())
                {
                    // Last parent reached
                    break;
                }
                for (const object_id_type& child : it_h->second)
                {
                    object_id_type parent = group(boundaries, child, it_l->second);
                    if (parent >= 0)
                    {
                        util::insert(hierarchy, parent, child);
                    }
                }
            }

            return hierarchy;
        }

    };

}
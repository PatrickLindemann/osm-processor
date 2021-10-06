#pragma once

#include <stack>

#include "model/memory/area.hpp"
#include "model/memory/node.hpp"
#include "model/memory/buffer.hpp"
#include "model/graph/undirected_graph.hpp"
#include "model/geometry/point.hpp"
#include "model/memory/ring.hpp"
#include "util/functions.hpp"

using namespace model;

namespace mapmaker
{

    namespace compressor
    {

        namespace detail
        {

            /* Definitions */
            
            using NodeRef     = memory::EntityRef<memory::Node>;
            using NodeList    = memory::EntityList<memory::Node>;
            using NodeRefList = memory::EntityRefList<memory::Node>;

            using BitVector     = std::vector<bool>;
            using IdentifierMap = std::unordered_map<memory::object_id_type, memory::object_id_type>;

            /* Functions */

            /**
             * Compresses a list of points with the Douglas-Peucker-Algorithm.
             * This method implements the iterative version of the algorithm,
             * as the recursive method initializes multiple new collections
             * that will be destroyed by the garbage collector anyway.
             * 
             * More information on the original algorithm can be found here:
             * https://en.wikipedia.org/wiki/Ramer%E2%80%93Douglas%E2%80%93Peucker_algorithm
             *
             * @param result
             * @param nodes
             * @param graph
             * @param epsilon
             * 
             * Time complexity: Linearithmic (Average-case),
             *                  Quadratic (Worst-case)
             */
            inline void douglas_peucker(
                std::vector<bool>& result,
                const NodeList& nodes,
                const graph::Graph& graph,
                const double epsilon
            ) {
                // Create the index stack for the iterative version
                // of the algorithm
                std::stack<std::pair<int, int>> stack;
                stack.push(std::make_pair(0, nodes.size() - 1));

                while (!stack.empty())
                {
                    // Get the current start and end index
                    auto [start, end] = stack.top();
                    stack.pop();

                    // Find the node with the greatest perpendicular distance to
                    // the line between the current start and end node
                    int index = start;
                    double d_max = 0.0;
                    for (int i = start + 1; i < end; i++)
                    {
                        // Check if node was removed already in another compression
                        // iteration
                        if (result.at(nodes.at(i).id()))
                        {
                            double d = util::perpendicular_distance(
                                nodes.at(i).point(),
                                nodes.at(start).point(), 
                                nodes.at(end).point()
                            );
                            if (d > d_max)
                            {
                                index = i;
                                d_max = d;
                            }
                        }
                    }

                    // Check if the maximum distance is greater than the upper threshold
                    if (d_max > epsilon)
                    {
                        // Compress the left and right part of the polyline
                        stack.push(std::make_pair(start, index));
                        stack.push(std::make_pair(index, end));
                    }
                    else
                    {
                        // Remove all nodes from the current polyline that are between the
                        // start and end node, except nodes with degree > 2
                        for (int i = start + 1; i < end; i++) {
                            if (graph.degree(nodes.at(i).id()) < 3)
                            {
                                result.at(nodes.at(i).id()) = 0;
                            }
                        }
                    }
                }
            }

            /**
             * 
             */
            inline void reindex(
                memory::Ring& ring,
                graph::UndirectedGraph& graph,
                const BitVector& kept_indices,
                const IdentifierMap& id_map
            ) {
                // Prepare the result object
                memory::Ring::container_type new_nodes;
                // Create the new ring and add the new resulting edges
                // into the graph
                new_nodes.push_back(id_map.at(ring.get(0)));
                for (size_t i = 0, j = 1; j < ring.size(); j++)
                {
                    const auto& ref = ring.get(j);
                    if (kept_indices.at(ref))
                    {
                        // Map the reference from the old node buffer to a reference
                        // in the new node buffer
                        memory::object_id_type new_ref = id_map.at(ref);
                        // Add the new reference to the new area ring
                        new_nodes.push_back(new_ref);
                        // Add new graph edge to the last existing node reference
                        memory::object_id_type last_ref = id_map.at(ring.get(i));
                        graph.insert_edge(graph::Graph::edge_type{
                            last_ref, new_ref
                        });
                        i = j;
                    }
                }
                std::swap(ring.nodes(), new_nodes);
            }

            /**
             * 
             * @param area_buffer
             * @param node_buffer
             * @param graph
             * @param removed_nodes
             * 
             * Time complexity: // TODO
             */
            inline void reindex(
                memory::Buffer<memory::Node>& node_buffer,
                memory::Buffer<memory::Area>& area_buffer,
                graph::UndirectedGraph& graph,
                const BitVector& kept_indices
            ) {
                // Prepare the result objects
                memory::Buffer<memory::Node> new_node_buffer;
                memory::Buffer<memory::Area> new_area_buffer;
                graph::UndirectedGraph new_graph;

                // Prepare the index map that maps the old node buffer indices to
                // the new node buffer
                IdentifierMap id_map;

                // Create a new node buffer with the old buffer while ignoring removed
                // nodes
                memory::object_id_type new_id = 0;
                for (const auto& node : node_buffer)
                {
                    if (kept_indices.at(node.id()))
                    {
                        id_map[node.id()] = new_id;
                        new_node_buffer.append(memory::Node{ new_id, node.point() });
                        ++new_id;
                    }
                }

                // Reindex the area buffer (in-place) and add the new graph edges
                for (const auto& area : area_buffer)
                {
                    // Create the new area. As no areas are removed by the compressor,
                    // the area and ring ids can stay the same
                    memory::Area new_area{
                        area.id(),
                        area.name(),
                        area.type(),
                        area.level()
                    };
                    // Create the area rings with the new node buffer
                    for (auto& outer_ring : area.outer_rings())
                    {
                        memory::Ring new_outer = outer_ring;
                        reindex(
                            new_outer,
                            new_graph,
                            kept_indices,
                            id_map
                        );
                        new_area.add_outer(new_outer);
                        for (const auto& inner_ring : area.inner_rings(outer_ring))
                        {
                            memory::Ring new_inner = inner_ring;
                            reindex(
                                new_inner,
                                new_graph,
                                kept_indices,
                                id_map
                            );
                            new_area.add_inner(new_outer, new_inner);
                        }
                    }
                    new_area_buffer.append(new_area);
                }
                
                // Swap the old buffer and graph references with the result
                // buffers and graph
                std::swap(node_buffer, new_node_buffer);
                std::swap(area_buffer, new_area_buffer);
                std::swap(graph, new_graph);
            }

            /**
             * 
             * @param result
             * @param refs
             * @param node_buffer
             * @param graph
             * @param epsilon
             * 
             * Time complexity: // TODO
             */
            inline void compress(
                BitVector& kept_indices,
                NodeRefList& ref_list,
                memory::Buffer<memory::Node>& node_buffer,
                graph::UndirectedGraph& graph,
                double epsilon
            ) {
                // Retrieve nodes from buffer
                NodeList nodes;
                for (const auto& ref : ref_list)
                {
                    // Filter nodes that were removed already to avoid
                    // multiple compression iterations for the same line
                    // segements 
                    if (kept_indices.at(ref))
                    {
                        nodes.push_back(node_buffer.get(ref));
                    }
                }

                // Compress the NodeList with the Douglas-Peucker-Algorithm.
                douglas_peucker(kept_indices, nodes, graph, epsilon);
            }
        }

        /* Functions */

        /**
         * 
         * @param area_buffer
         * @param node_buffer
         * @param graph
         * @param epsilon
         * 
         * Time complexity: // TODO
         */
        inline void compress(
            memory::Buffer<memory::Node>& node_buffer,
            memory::Buffer<memory::Area>& area_buffer,
            graph::UndirectedGraph& graph,
            double epsilon
        ) {
            detail::BitVector kept_indices(node_buffer.size(), 1);
            for (auto& area : area_buffer)
            {
                for (auto& outer : area.outer_rings())
                {
                    // Compress outer ring
                    detail::compress(kept_indices, outer.nodes(), node_buffer, graph, epsilon);
                    for (auto& inner : area.inner_rings(outer))
                    {
                        // Compress inner ringss
                        detail::compress(kept_indices, inner.nodes(), node_buffer, graph, epsilon);
                    }
                }
            }
            detail::reindex(node_buffer, area_buffer, graph, kept_indices);
        }

    }

}
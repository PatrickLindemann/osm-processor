#pragma once

#include <cassert>
#include <stack>
#include <unordered_map>

#include "model/geometry/point.hpp"
#include "model/memory/node.hpp"
#include "model/memory/way.hpp"
#include "model/memory/buffer.hpp"
#include "functions/distance.hpp"

using namespace model;

namespace mapmaker
{

    namespace compressor
    {

        template <typename T>
        class Compressor
        {
        public:

            using id_type = memory::object_id_type;
            using id_map  = std::unordered_map<id_type, id_type>;

        protected:

            memory::Buffer<memory::Node<T>>& m_nodes;
            memory::Buffer<memory::Way<T>>& m_ways;
            double m_epsilon;

            /**
             *  The compression result vector of node indices that indicates
             *  which nodes should be kept or removed.
             *  If kept_indices[i] == true, the node with index i will be kept,
             *  else it will be removed.
             */
            std::vector<bool> m_kept_indices;

            /**
             * The lookup vector for node degrees.
             * These nodes will be ignored by the compressor in order
             * to prevent different compressions of node segments
             */
            std::vector<size_t> m_degrees;

        public:

            Compressor(
                memory::Buffer<memory::Node<T>>& nodes,
                memory::Buffer<memory::Way<T>>& ways,
                double epsilon
            ) : m_nodes(nodes), m_ways(ways), m_epsilon(epsilon),
                m_kept_indices(nodes.size(), true), m_degrees(nodes.size())
            {
                // Prepare node degree lookup vector
                for (const memory::Way<T>& way : m_ways)
                {
                    for (const id_type& node_ref : way.nodes())
                    {
                        m_degrees.at(node_ref)++;
                    }
                }
            };

        protected:
            
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
             * Time complexity: Linear (Average-case), Quadratic (Worst-case)
             */
            inline void douglas_peucker(
                const std::vector<memory::Node<T>>& nodes
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
                        // Check if node was removed already in another
                        // iteration
                        if (m_kept_indices.at(nodes.at(i).id()))
                        {
                            double d = functions::perpendicular_distance(
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
                    if (d_max > m_epsilon)
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
                            if (m_degrees.at(nodes.at(i).id()) < 3)
                            {
                                m_kept_indices.at(nodes.at(i).id()) = false;
                            }
                        }
                    }
                }
            }

        public:

            void run()
            {
                // Compress the ways according to the Douglas-Peucker-Algorithm
                for (const memory::Way<T>& way : m_ways)
                {
                    // Retrieve the referenced way nodes from the buffer
                    std::vector<memory::Node<T>> nodes;
                    for (const id_type& node_ref : way.nodes())
                    {
                        // Filter nodes that were removed already to avoid
                        // multiple compression iterations for the same line
                        // segements 
                        if (m_kept_indices.at(node_ref))
                        {
                            nodes.push_back(m_nodes.get(node_ref));
                        }
                    }
                    douglas_peucker(nodes);
                }
                
                // Prepare the result buffers that contain the compressed,
                // reindexed nodes
                memory::Buffer<memory::Node<T>> compressed_nodes;
                memory::Buffer<memory::Way<T>> compressed_ways;

                // Prepare the index map that maps the current node indices to
                // compressed node indices
                id_map map;

                // Create the new node buffer by adding all nodes from the old
                // buffer that have not been marked as removed
                for (const memory::Node<T>& node : m_nodes)
                {
                    if (m_kept_indices.at(node.id()))
                    {
                        id_type new_id = map.size();
                        map[node.id()] = new_id;
                        compressed_nodes.append(memory::Node<T>{ new_id, node.point() });
                    }
                }

                // Create the new way buffer by re-creating the ways with the nodes
                // that have not been marked as removed
                for (const memory::Way<T>& way : m_ways)
                {
                    memory::Way<T> compressed_way{ way.id() };
                    for (const id_type& node_ref : way.nodes())
                    {
                        if (m_kept_indices.at(node_ref))
                        {
                            compressed_way.nodes().push_back(map.at(node_ref));
                        }
                    }
                    compressed_ways.append(compressed_way);
                }
                
                // Swap the old buffer and graph references with the result
                // buffers and graph
                std::swap(m_nodes, compressed_nodes);
                std::swap(m_ways, compressed_ways);
            }

        };

    }

}
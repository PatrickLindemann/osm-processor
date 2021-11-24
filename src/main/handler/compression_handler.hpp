#pragma once

#include <set>

#include <osmium/handler.hpp>
#include <osmium/osm/way.hpp>
#include <osmium/osm/node_ref.hpp>
#include <osmium/osm/node_ref_list.hpp>

#include "functions/distance.hpp"
#include "model/geometry/point.hpp"

namespace handler
{

    /**
     * A handler that counts the total number of nodes, ways and
     * relations of an osmium object stream.
     *
     * Port of https://github.com/osmcode/libosmium/blob/master/examples/osmium_count.cpp
     */
    class CompressionHandler : public osmium::handler::Handler
    {
    protected:

        /* Members */

        /**
         *
         */
        double m_tolerance;

        /**
         *
         */
        std::set<osmium::object_id_type> m_ignored_nodes;

        /**
         *  The compression result array of node indices that indicates
         *  which nodes should be kept or removed.
         */
        std::set<osmium::object_id_type> m_removed_nodes;

    public:

        /* Constructors */

        CompressionHandler(double tolerance) : m_tolerance(tolerance) {}

        CompressionHandler(double tolerance, const std::set<osmium::object_id_type>& ignored_nodes)
            : m_tolerance(tolerance), m_ignored_nodes(ignored_nodes) {}

        /* Accessors */

        const std::set<osmium::object_id_type>& removed_nodes() const
        {
            return m_removed_nodes;
        };

    protected:

        /* Helper Methods */

        /**
        * Compresses a list of nodes with the Douglas-Peucker-Algorithm.
        * This method implements the iterative version of the algorithm,
        * as the recursive method initializes multiple new collections
        * that will be destroyed by the garbage collector anyway.
        *
        * For more information on the original algorithm, refer to
        * https://en.wikipedia.org/wiki/Ramer%E2%80%93Douglas%E2%80%93Peucker_algorithm
        *
        * @param nodes     The node reference list
        * @param tolerance The compression distance tolerance (epsilon)
        *
        * Time complexity: Log-Linear (Average-case), Quadratic (Worst-case)
        */
        inline void douglas_peucker(const osmium::NodeRefList& nodes, double tolerance)
        {
            // Create the index stack for the iterative version
            // of the algorithm
            std::stack<std::pair<std::size_t, std::size_t>> stack;
            stack.push(std::make_pair(0, nodes.size() - 1));

            while (!stack.empty())
            {
                // Get the current start and end index
                auto [start, end] = stack.top();
                stack.pop();

                // Find the node with the greatest perpendicular distance to
                // the line between the current start and end node
                std::size_t index = start;
                double d_max = 0.0;
                for (std::size_t i = start + 1; i < end; i++)
                {
                    // Check if node was removed already in another
                    // iteration
                    if (!m_removed_nodes.count(nodes[i].ref()))
                    {
                        double d = functions::perpendicular_distance(
                            model::geometry::Point{ nodes[i].lon(), nodes[i].lat() },
                            model::geometry::Point{ nodes[start].lon(), nodes[start].lat() },
                            model::geometry::Point{ nodes[end].lon(), nodes[end].lat() }
                        );
                        if (d > d_max)
                        {
                            index = i;
                            d_max = d;
                        }
                    }
                }

                // Check if the maximum distance is greater than the upper tolerance
                if (d_max > tolerance)
                {
                    // Compress the left and right part of the polyline
                    stack.push(std::make_pair(start, index));
                    stack.push(std::make_pair(index, end));
                }
                else
                {
                    // Remove all nodes from the current polyline that are between the
                    // start and end node, except nodes with degree > 2
                    for (std::size_t i = start + 1; i < end; i++)
                    {
                        osmium::object_id_type n_id = nodes[i].ref();
                        if (!m_ignored_nodes.count(n_id))
                        {
                            m_removed_nodes.insert(n_id);
                        }
                    }
                }
            }
        }

    public:

        /* Osmium Methods */

        void way(const osmium::Way& way) noexcept
        {
            douglas_peucker(way.nodes(), m_tolerance);
        }

    };

}
#pragma once

#include <functional>
#include <stack>
#include <iostream>

#include "model/geometry/polygon.hpp"
#include "model/memory/area.hpp"
#include "model/memory/node.hpp"
#include "model/memory/buffer.hpp"
#include "model/graph/graph.hpp"
#include "model/graph/undirected_graph.hpp"
#include "model/geometry/point.hpp"
#include "model/geometry/rectangle.hpp"
#include "util/functions.hpp"

using namespace model;

namespace mapmaker
{

    namespace algorithm
    {

        namespace detail
        {

            /* Constants */

            const double SQRT_TWO = std::sqrt(2);

            template <class T>
            class Cell {
            public:

                /* Types */

                using point_type   = geometry::Point<T>;
                using polygon_type = geometry::Polygon<T>;

                /* Members */

                point_type center;
                T half;
                T distance;
                T max;

                /* Constructors */

                Cell(const point_type& center, T half, T distance, T max)
                : center(center), half(half), distance(distance), max(max) {};

                Cell(const point_type& center, T half, const polygon_type& polygon)
                : center(center), half(half)
                {
                    distance = util::distance_to_polygon(center, polygon),
                    max = distance + half * SQRT_TWO;
                };

            };

            /**
             *
             */
            template <typename T>
            Cell<T> get_centroid_cell(const geometry::Polygon<T>& polygon)
            {
                T area = 0;
                geometry::Point<T> center{ 0, 0 };
                for (size_t i = 0, j = 1; j < polygon.outer.size(); i++, j++)
                {
                    const geometry::Point<T>& a = polygon.outer.at(i);
                    const geometry::Point<T>& b = polygon.outer.at(j);
                    auto f = a.x * b.y - b.x * a.y;
                    center.x += (a.x + b.x) * f;
                    center.y += (a.y + b.y) * f;
                    area += f * 3;
                }
                if (area > 0)
                {
                    center /= area;
                    return Cell<T>{ center, 0, polygon } ;
                }
                return Cell<T>{ polygon.outer.at(0), 0, polygon };
            }

        }

        /* Definitions */

        using Graph  = graph::Graph;
        using Vertex = Graph::vertex_type;
        using Edge   = Graph::edge_type;

        /* Functions */

        /**
         * Calculates the envelope of a polygon.
         *
         * @param points The list of points
         * @return The axis-oriented minimal bounding box that encloses the polygon.
         */
        template <typename T>
        geometry::Rectangle<T> envelope(const geometry::Polygon<T>& polygon)
        {
            std::numeric_limits<T> limits;
            T min_x = limits.max(), min_y = limits.max() ;
            T max_x = limits.lowest(), max_y = limits.lowest();
            for (const geometry::Point<T>& p : polygon.outer)
            {
                min_x = std::min(min_x, p.x);
                min_y = std::min(min_y, p.y);
                max_x = std::max(max_x, p.x);
                max_y = std::max(max_y, p.y);
            }
            return geometry::Rectangle<T>( { min_x, min_y }, { max_x, max_y } );
        }

        /**
         * Calculate the centroid of a rectangle.
         *
         * @param points The rectangle
         * @return The centroid as Point
         */
        template <typename T>
        inline geometry::Point<T> center(const geometry::Rectangle<T>& rectangle)
        {
            return geometry::Point<T>{
                rectangle.min.x + rectangle.width() / 2,
                rectangle.min.y + rectangle.height() / 2
            };
        }

        /**
         * Calculate the centroid of a polygon by calculating the
         * weigted sum of all points.
         *
         * NOTE: This algorithm does not return "ideal" centerpoints.
         *
         * @param poylgon The polygon
         * @return The centroid as Point
         */
        // template <typename T>
        // inline geometry::Point<T> center(const geometry::Polygon<T>& polygon)
        // {
        //     geometry::Point<T> center;
        //     for (const auto& point : polygon.outer())
        //     {
        //         center += point;
        //     }
        //     center /= polygon.outer().size();
        //     return center;
        // }

        /**
         *
         */
        template <typename T>
        geometry::Point<T> center(const geometry::Polygon<T>& polygon, int precision = 1)
        {
            using Cell = detail::Cell<T>;

            // Calculate the polygon envelope, which is the minimal bounding
            // box that enclosed the outer ring
            const geometry::Rectangle<T> polygon_envelope = envelope(polygon);

            // Scale the cells according to the envelope
            const T cell_size = std::min(polygon_envelope.width(), polygon_envelope.height());
            if (cell_size == 0)
            {
                return polygon_envelope.min;
            }
            T half = cell_size / 2;

            // Prepare the priority queue
            auto compare = [](const Cell& a, const Cell& b)
            {
                return a.max < b.max;
            };
            std::priority_queue<Cell, std::vector<Cell>, decltype(compare)> queue(compare);

            // Cover the polygon with the initial cells
            for (T x = polygon_envelope.min.x; x < polygon_envelope.max.x; x += cell_size)
            {
                for (T y = polygon_envelope.min.y; y < polygon_envelope.max.y; y += cell_size)
                {
                    queue.push(Cell({ x + half, y + half }, half, polygon));
                }
            }

            // take centroid as the first best guess
            Cell best_cell = detail::get_centroid_cell(polygon);

            // second guess: bounding box centroid
            Cell envelope_cell(center(polygon_envelope), 0, polygon);
            if (envelope_cell.distance > best_cell.distance)
            {
                best_cell = envelope_cell;
            }

            while (!queue.empty())
            {
                // Pick the most promising cell from the top of the queue
                Cell cell = queue.top();
                queue.pop();

                // Update the best cell if another one is found
                if (cell.distance > best_cell.distance)
                {
                    best_cell = cell;
                }

                // Check if there potentially is a better solution
                if (cell.max - best_cell.distance <= precision)
                {
                    // Split the current cell into 4 cells and add them to the queue
                    half = cell.half / 2;
                    queue.push(Cell({ cell.center.x + half, cell.center.y + half }, half, polygon));
                    queue.push(Cell({ cell.center.x + half, cell.center.y - half }, half, polygon));
                    queue.push(Cell({ cell.center.x - half, cell.center.y + half }, half, polygon));
                    queue.push(Cell({ cell.center.x - half, cell.center.y - half }, half, polygon));
                }
            }

            return best_cell.center;
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
        inline void get_vertices_for_component(
            std::vector<long>& result,
            size_t& count,
            const Vertex& vertex,
            const int component,
            const Graph& graph
        ) {
            // Mark the current vertex as part of the current
            // component
            result[vertex] = component;
            // Mark all adjacents of this vertex if they haven't
            // been visited already
            for (const Vertex& adjacent : graph.adjacents(vertex))
            {
                if (result[adjacent] < 0)
                {
                    // Vertex adjacent wasn't visited before, perform dfs
                    // recursively
                    get_vertices_for_component(result, count, adjacent, component, graph);
                    ++count;
                }
            }
        }

        /**
         *
         */
        inline std::pair<std::vector<size_t>, std::vector<long>> get_components(const Graph& graph)
        {
            // Check for empty graphs
            if (graph.vertex_count() == 0) {
                return std::make_pair(
                    std::vector<size_t>{},
                    std::vector<long>{}
                );
            }

            // Create the component vector which index maps to the component
            // id and the value maps to the number of vertices contained in
            // this component.
            std::vector<size_t> components{};

            // Create the vertex to component vector which index maps to the
            // vertex id and the value maps to the component it is contained in.
            std::vector<long> vertex_components(graph.vertex_count(), -1);

            // The current component id
            int component = 0;
            // The current vertex count
            size_t vertex_count = 0;

            //
            for (Vertex vertex = 0; vertex < graph.vertex_count(); vertex++)
            {
                // Check if the current vertex was visited already
                if (vertex_components[vertex] < 0)
                {
                    // Mark this vertex and all vertices it is directly and
                    // indirectly connected to as part of the current component
                    // by perform a dfs.
                    get_vertices_for_component(vertex_components, vertex_count, vertex, component, graph);
                    // The component is finished, save the vertex count and prepare
                    // the next one
                    components.push_back(vertex_count);
                    vertex_count = 0;
                    ++component;
                }
            }
            // Return the resulting component vector.
            return std::make_pair(components, vertex_components);

        }

    }

}
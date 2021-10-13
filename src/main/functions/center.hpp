#pragma once

#include <cmath>

#include "model/geometry/point.hpp"
#include "model/geometry/rectangle.hpp"
#include "model/geometry/polygon.hpp"
#include "model/geometry/multipolygon.hpp"
#include "util/functions.hpp"

using namespace model;

namespace functions
{

    namespace detail
    {

        /* Constants */

        const double SQRT_TWO = std::sqrt(2);

        /* Classes */

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

            /**
             * 
             * @param center
             * @param half
             * @param distance
             * @param max
             */
            Cell(const point_type& center, T half, T distance, T max)
            : center(center), half(half), distance(distance), max(max) {};

            /**
             * 
             * @param center
             * @param half
             * @param polygon
             */
            Cell(const point_type& center, T half, const polygon_type& polygon)
            : center(center), half(half)
            {
                distance = util::distance_to_polygon(center, polygon),
                max = distance + half * SQRT_TWO;
            };

        };

        /* Functions */

        /**
         *
         * @param polygon
         * @returns
         */
        template <typename T>
        inline Cell<T> get_centroid(const geometry::Polygon<T>& polygon)
        {
            T area = 0;
            geometry::Point<T> center{ 0, 0 };
            for (size_t i = 0, j = 1; j < polygon.outer.size(); i++, j++)
            {
                const geometry::Point<T>& left = polygon.outer.at(i);
                const geometry::Point<T>& right = polygon.outer.at(j);
                auto f = left.x * right.y - right.x * left.y;
                center.x += (left.x + right.x) * f;
                center.y += (left.y + right.y) * f;
                area += f * 3;
            }
            if (area > 0)
            {
                center /= area;
                return Cell<T>{ center, 0, polygon } ;
            }
            return Cell<T>{ polygon.outer.at(0), 0, polygon };
        }

        /**
         *
         */
        template <typename T>
        inline std::pair<geometry::Point<T>, double> point_of_isolation(
            const geometry::Polygon<T>& polygon,
            double precision = 1
        ) {
            using Cell = detail::Cell<T>;

            // Calculate the polygon envelope, which is the minimal bounding
            // box that enclosed the outer ring
            const geometry::Rectangle<T> polygon_envelope = envelope(polygon);

            // Scale the cells according to the envelope
            const T cell_size = std::min(polygon_envelope.width(), polygon_envelope.height());
            if (cell_size == 0)
            {
                return std::make_pair(polygon_envelope.min, 0);
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
            Cell best_cell = detail::get_centroid(polygon);
            
            // second guess: bounding box centroid
            Cell envelope_center_cell(center(polygon_envelope), 0, polygon);
            if (envelope_center_cell.distance > best_cell.distance)
            {
                best_cell = envelope_center_cell;
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
                    continue;
                }

                // Split the current cell into 4 cells and add them to the queue
                half = cell.half / 2;
                queue.push(Cell({ cell.center.x + half, cell.center.y + half }, half, polygon));
                queue.push(Cell({ cell.center.x + half, cell.center.y - half }, half, polygon));
                queue.push(Cell({ cell.center.x - half, cell.center.y + half }, half, polygon));
                queue.push(Cell({ cell.center.x - half, cell.center.y - half }, half, polygon));

            }

            return std::make_pair(best_cell.center, best_cell.distance);
        }

    }

    /**
     * Calculate the centerpoint of a rectangle.
     *
     * @param rectangle The rectangle
     * @return A pair of the center point and its distance 
     */
    template <typename T>
    inline std::pair<geometry::Point<T>, long> center(const geometry::Rectangle<T>& rectangle)
    {  
        double half_width = rectangle.width() / 2;
        double half_height = rectangle.height() / 2;
        return std::make_pair(
            { rectangle.min.x + half_width, rectangle.min.y + half_height },
            std::min(half_width, half_height)
        );
    }

    /**
     * Calculate the center point of a polygon, which is the point that has
     * the maximum distance to the polygon sides (point of isolation).
     *
     * @param polygon The polygon.
     * @return The center point of the polygon.
     */
    template <typename T>
    inline std::pair<geometry::Point<T>, long> center(const geometry::Polygon<T>& polygon)
    {
        return detail::point_of_isolation(polygon);
    }

    /**
     * Calculate the center point of a polygon, which is the point that has
     * the maximum distance inside any of the polygons (point of isolation).
     *
     * @param multipolygon The multipolygon.
     * @return The center point of the multipolygon.
     */
    template <typename T>
    inline std::pair<geometry::Point<T>, long> center(const geometry::MultiPolygon<T>& multipolygon)
    {
        // Prepare the result
        geometry::Point<T> center = { 0, 0 };
        double distance = 0;
        // Search for the maximum point of isolation in the polygons
        for (const geometry::Polygon<T>& polygon : multipolygon.polygons)
        {
            auto [c, d] = detail::point_of_isolation(polygon);
            if (distance < d)
            {
                center = c;
                distance = d;
            }
        }
        return std::make_pair(center, distance);
    }

}
#pragma once

#include <cmath>
#include <queue>
#include <utility>

#include "functions/area.hpp"
#include "functions/envelope.hpp"
#include "functions/distance.hpp"
#include "functions/util.hpp"
#include "model/geometry/point.hpp"
#include "model/geometry/rectangle.hpp"
#include "model/geometry/polygon.hpp"
#include "model/geometry/multipolygon.hpp"
#include "model/geometry/ring.hpp"

using namespace model;

namespace functions
{

    namespace detail
    {

        /**
         * 
         */
        template <typename T>
        inline std::pair<geometry::Point<T>, double> center_and_area(
            const geometry::Ring<T>& ring,
            const std::string winding = "ccw"
        ){
            // Prepare the result centroid
            geometry::Point<T> center{ 0, 0 };
            // Check if the ring is valid
            if (ring.size() < 2)
            {
                return std::make_pair(center, 0.0);
            }
            // Calculate the center of the ring according to its winding
            double area = 0.0;
            double f = 0.0;
            const geometry::Point<T> first = ring.front();
            if (winding == "cw")
            {
                for (size_t i = ring.size() - 1; i > 0; i--)
                {
                    const geometry::Point<T>& p1 = ring.at(i);
                    const geometry::Point<T>& p2 = ring.at(i - 1);
                    f = (p1.x() - first.x()) * (p2.y() - first.y())
                    - (p1.y() - first.y()) * (p2.x() - first.x());
                    area += f;
                    center.x() += (p1.x() + p2.x() - 2 * first.x()) * f;
                    center.y() += (p1.y() + p2.y() - 2 * first.y()) * f;
                }
            }
            else
            {
                for (size_t i = 0; i < ring.size() - 1; i++)
                {
                    const geometry::Point<T>& p1 = ring.at(i);
                    const geometry::Point<T>& p2 = ring.at(i + 1);
                    f = (p1.x() - first.x()) * (p2.y() - first.y())
                    - (p1.y() - first.y()) * (p2.x() - first.x());
                    area += f;
                    center.x() += (p1.x() + p2.x() - 2 * first.x()) * f;
                    center.y() += (p1.y() + p2.y() - 2 * first.y()) * f;
                }
            }
            // Calculate the resulting area
            area *= 0.5;
            // Remove the area factor from the center
            if (area > 0)
            {
                center /= area * 6;
            }
            // Remove the first point from the center
            center += first;
            // Return the calculated centroid and area
            return std::make_pair(center, area);
        }

        /**
         * 
         */
        template <typename T>
        inline std::pair<geometry::Point<T>, double> center_and_area(const geometry::Polygon<T>& polygon)
        {
            // Prepare the result center point
            geometry::Point<T> center;
            double area = 0.0;
            // Calculate the centerpoint and surface area of the outer ring,
            // weigh the center by the surface area and add it to the result
            auto [outer_center, outer_area] = center_and_area(polygon.outer(), "ccw");
            center += outer_center * outer_area;
            // Repeat this process for each inner ring of the polygon
            double inner_areas = 0.0;
            for (const geometry::Ring<T>& inner : polygon.inners())
            {
                auto [inner_center, inner_area] = center_and_area(inner, "cw");
                center += inner_center * inner_area;
                inner_areas += inner_area;
            }
            // Remove the area weights from the center point
            double area_sum = outer_area + inner_areas;
            if (area_sum > 0)
            {
                center /= area_sum;
            }
            // Return the resulting centroid and surface area
            return std::make_pair(center, outer_area - inner_areas);
        }
        
    }

    /* Functions */

    /**
     * Calculate the centerpoint of a rectangle.
     *
     * @param rectangle The rectangle
     * @return A pair of the center point and its distance 
     */
    template <typename T>
    inline geometry::Point<T> center(const geometry::Rectangle<T>& rectangle)
    {  
        double half_width = rectangle.width() / 2;
        double half_height = rectangle.height() / 2;
        geometry::Point<T> center{
            rectangle.min().x() + half_width,
            rectangle.min().y() + half_height
        };
        return std::make_pair(center, std::min(half_width, half_height));
    }

    /**
     * Calculate the center point of a ring, which is the point
     * the weighted sum of all points in the ring.
     * 
     * Note: This algorithm does not provide the (optimal) point of
     * isolation, but the approximation of it is enough for our case.
     *
     * @param ring The ring
     * @return     The center point of the ring
     */
    template <typename T>
    inline geometry::Point<T> center(const geometry::Ring<T>& ring, const std::string winding = "ccw")
    {
        auto [center, area] = detail::center_and_area(ring, winding);
        return center;
    }

    /**
     * Calculate the center point of a polygon, which is the point
     * the weighted sum of all center points of each ring weighted
     * with the respective ring area.
     *
     * @param polygon The polygon.
     * @return        The center point of the polygon.
     */
    template <typename T>
    inline geometry::Point<T> center(const geometry::Polygon<T>& polygon)
    {
        auto [center, area] = detail::center_and_area(polygon);
        return center;
    }

    /**
     * Calculate the center point of a polygon, which is the point that has
     * the maximum distance inside any of the polygons (point of isolation).
     *
     * @param multipolygon The multipolygon.
     * @return The center point of the multipolygon.
     */
    template <typename T>
    inline geometry::Point<T> center(const geometry::MultiPolygon<T>& multipolygon)
    {
        // Prepare the result center point
        geometry::Point<T> center;
        double area_sum = 0.0f;
        // Calculate the centerpoint and surface area of the polygon
        // weigh the center by the surface area and add it to the result
        for (const geometry::Polygon<T>& polygon : multipolygon.polygons())
        {
            auto [polygon_center, polygon_area] = detail::center_and_area(polygon);
            center += polygon_center * polygon_area;
            area_sum += polygon_area;
        }
        // Remove the area weights from the center point
        if (area_sum > 0)
        {
            center /= area_sum;
        }
        // Return the resulting centroid
        return center;
    }

}
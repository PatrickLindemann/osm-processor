#pragma once

#include <cfloat>
#include <cmath>
#include <queue>

#include "model/geometry/point.hpp"
#include "model/geometry/polygon.hpp"
#include "model/geometry/rectangle.hpp"
#include "model/geometry/ring.hpp"

using namespace model;

namespace util
{

    /**
     * Calculate the dot product of two points
     * 
     * @param p The first point
     * @param q The second point
     * @return The dot product of p and q
     * 
     * Time complexity: Constant
     */
    template <typename T>
    inline double dot(
        const geometry::Point<T>& p,
        const geometry::Point<T>& q
    ) {
        return p.x * q.x + p.y * q.y;
    }

    /**
     * Calculate the distance between two points
     * 
     * @param p The first point
     * @param q The second point
     * @return The absolute distance between p and q
     * 
     * Time complexity: Constant
     */
    template <typename T>
    inline double distance(
        const geometry::Point<T>& p,
        const geometry::Point<T>& q
    ) {
        return std::hypot(p.x - q.x, p.y - q.y);
    }

    /**
     * Calculate the perpendicular distance of a point to a segment
     * 
     * @param point The point
     * @param line_start The line starting point
     * @param line_end The line ending point
     * @return The perpendicular distance of the point to the segment
     * 
     * Time complexity: Constant
     */
    template <typename T>
    inline double perpendicular_distance(
        const geometry::Point<T>& point,
        const geometry::Point<T>& line_start,
        const geometry::Point<T>& line_end
    ) {
        // Calculate the direction vector of the line
        geometry::Point<T> dir = line_end - line_start;

        // Normalize the direction vector
        double length = std::hypot(dir.x, dir.y);
        if (length > 0.0)
        {
            dir /= length;
        }
            
        // Calculate the point-to-line vector
        geometry::Point<T> ps = point - line_start;

        // Calculate the dot product between the two points to retrieve
        // the length between line_start and the plumb point L relative to p
        double dt = dot(dir, ps);

        // Calculate the point of plumb on the line relative to p
        // by scaling the line direction vector
        geometry::Point<T> line_plumb = dir * dt;

        // Calculate the vector from the plumb point and p and return its distance
        return distance(ps, line_plumb);
    }

    /**
     * 
     * @param point
     * @param ring
     */
    template <typename T>
    inline double distance_to_ring(
        const geometry::Point<T>& point,
        const geometry::Ring<T>& ring
    ) {
        // Check for invalid rings
        if (ring.size() < 3)
        {
            // TODO throw error
        }

        double result = DBL_MAX;
        bool inside = true;

        // Iterate over the ring segments and determine the minimum
        // distance between the point and any segment
        for (size_t i = 0, j = 1; j < ring.size(); i++, j++) {
            const auto& left = ring.at(i);
            const auto& right = ring.at(j);

            // Check if point is inside or outside of the ring
            if ((left.y > point.y) != (right.y > point.y))
            {
                double x = (right.x - left.x) + left.x;
                double y = (point.y - left.y) / (right.y - left.y);
                if (point.x < x * y)
                {
                    inside = !inside;
                }
            }

            // Determine the perpendicular distance to the current segment
            // and save it if it is the new minimum
            double d = perpendicular_distance(point, left, right);
            if (d < result)
            {
                result = d;
            }
        }

        return -1 * !inside * std::sqrt(result);
    }

    /**
     * Calculate the minimal distance of a point to the outer ring of 
     * a polygon.
     * 
     * @param point The point
     * @param polygon The polygon
     * @return The minimal (signed) distance from to point to the polygon.
     * 
     * Time complexity: Constant
     */
    template <typename T>
    inline double distance_to_polygon(
        const geometry::Point<T>& point,
        const geometry::Polygon<T>& polygon
    ) {
        double result = distance_to_ring(point, polygon.outer);

        // Calculate the minimal (signed) distance of the point
        // to each inner ring of the polygon 
        for (const auto& inner : polygon.inners)
        {
            double d = distance_to_ring(point, inner);
            if (std::abs(d) < std::abs(result))
            {
                result = d;
            }    
        }

        return result;
    }

}
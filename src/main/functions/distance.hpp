#pragma once

#include <cmath>

#include "model/geometry/point.hpp"
#include "model/geometry/rectangle.hpp"
#include "model/geometry/ring.hpp"

#include "functions/util.hpp"

using namespace model::geometry;

namespace functions
{

    /**
     * Calculate the euclidean distance between two points.
     * 
     * @param point1 The first point
     * @param point2 The second point
     * @return       The absolute distance between the points
     * 
     * Time complexity: Constant
     */
    template <typename T>
    inline double distance(const Point<T>& p, const Point<T>& q)
    {
        return std::hypot(p.x() - q.x(), p.y() - q.y());
    }

    /**
     * Calculate the perpendicular distance of a point to a segment,
     * defined by two points.
     * 
     * @param point      The point
     * @param line_start The line starting point
     * @param line_end   The line ending point
     * @return           The perpendicular distance of the point to
     *                   the segment
     * 
     * Time complexity: Constant
     */
    template <typename T>
    inline double perpendicular_distance(const Point<T>& p, const Point<T>& s1, const Point<T>& s2)
    {
        // Calculate the direction vector of the segment
        Point<T> dir = s1 - s2;

        // Normalize the direction vector
        double length = std::hypot(dir.x(), dir.y());
        if (length > 0.0)
        {
            dir /= length;
        }
            
        // Calculate the point-to-line vector
        Point<T> ps = p - s1;

        // Calculate the dot product between the two points to retrieve
        // the length between segment_start and the plumb point L relative to p
        double dt = dot(dir, ps);

        // Calculate the point of plumb on the line relative to p
        // by scaling the line direction vector
        Point<T> line_plumb = dir * dt;

        // Calculate the vector from the plumb point and p and return its distance
        return distance(ps, line_plumb);
    }

    /**
     * Calculate the minimal (signed) distance of to a ring.
     * 
     * @param point     The point
     * @param ring      The ring
     * @returns         The signed distance with these properties:
     *                   d > 0 => Point is inside of the ring
     *                   d < 0 => Point is outside of the ring
     *                   d = 0 => Point is on the ring
     * 
     * Time complexity: Linear
     */
    template <typename T>
    inline double distance(const Point<T>& p, const Ring<T>& ring)
    {       
        bool inside;
        double distance = DBL_MAX;
        // Iterate over the ring segments and determine the minimum
        // distance between the point and any segment
        for (std::size_t i = 0; i < ring.size(); i++)
        {
            const Point<T>& left = ring.at(i);
            const Point<T>& right = ring.at(i + 1);
            // Check if point is inside or outside of the ring
            if ((left.y() > p.y()) != (right.y() > p.y()))
            {
                double f = (right.x() - left.x()) * (p.y() - left.y()) / (right.y() - left.y()) + left.x();
                if (p.x() < f)
                {
                    inside = !inside;
                }
            }
            // Determine the perpendicular distance to the current segment
            // and save it if it is the new minimum
            double d = perpendicular_distance(p, left, right);
            distance = std::min(d, distance);
        }
        return (inside ? 1 : -1) * std::sqrt(distance);
    }

}
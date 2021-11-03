#pragma once

#include <cfloat>
#include <cmath>

#include "model/geometry/point.hpp"
#include "model/geometry/rectangle.hpp"
#include "model/geometry/ring.hpp"
#include "model/geometry/polygon.hpp"
#include "functions/util.hpp"

using namespace model;

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
    inline double distance(
        const geometry::Point<T>& point1,
        const geometry::Point<T>& point2
    ) {
        return std::hypot(
            point1.x() - point2.x(),
            point1.y() - point2.y()
        );
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
    inline double perpendicular_distance(
        const geometry::Point<T>& point,
        const geometry::Point<T>& segment_start,
        const geometry::Point<T>& segment_end
    ) {
        // Calculate the direction vector of the segment
        geometry::Point<T> dir = segment_start - segment_end;

        // Normalize the direction vector
        double length = std::hypot(dir.x(), dir.y());
        if (length > 0.0)
        {
            dir /= length;
        }
            
        // Calculate the point-to-line vector
        geometry::Point<T> ps = point - segment_start;

        // Calculate the dot product between the two points to retrieve
        // the length between segment_start and the plumb point L relative to p
        double dt = dot(dir, ps);

        // Calculate the point of plumb on the line relative to p
        // by scaling the line direction vector
        geometry::Point<T> line_plumb = dir * dt;

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
    inline double distance_to_ring(
        const geometry::Point<T>& point,
        const geometry::Ring<T>& ring
    ) {       
        bool inside;
        double distance = DBL_MAX;
        // Iterate over the ring segments and determine the minimum
        // distance between the point and any segment
        for (size_t i = 0, j = ring.size() - 1; i < ring.size(); j = i++) {
            const geometry::Point<T>& left = ring.at(i);
            const geometry::Point<T>& right = ring.at(j);
            // Check if point is inside or outside of the ring
            if ((left.y() > point.y()) != (right.y() > point.y()))
            {
                double f = (right.x() - left.x()) * (point.y() - left.y()) / (right.y() - left.y()) + left.x();
                if (point.x() < f)
                {
                    inside = !inside;
                }
            }
            // Determine the perpendicular distance to the current segment
            // and save it if it is the new minimum
            double d = perpendicular_distance(point, left, right);
            distance = std::min(d, distance);
        }
        return (inside ? 1 : -1) * std::sqrt(distance);
    }

    /**
     * Calculate the minimal (signed) distance of a point to any
     * ring of a polygon.
     * 
     * @param point     The point
     * @param polygon   The polygon
     * @returns         The signed distance with these properties:
     *                   d > 0 => Point is inside of the polygon
     *                   d < 0 => Point is outside of the polygon
     *                   d = 0 => Point is on the polygon
     * 
     * Time complexity: Linear
     */
    template <typename T>
    inline double distance_to_polygon(
        const geometry::Point<T>& point,
        const geometry::Polygon<T>& polygon
    ) {
        double distance = distance_to_ring(point, polygon.outer());
        // Calculate the minimal (signed) distance of the point
        // to each inner ring of the polygon 
        for (const auto& inner : polygon.inners())
        {
            double d = distance_to_ring(point, inner);
            if (std::abs(d) < std::abs(distance))
            {
                distance  = d;
            }
        }
        return distance;
    }

    /**
     * Calculate the distance between two rectangles
     * 
     * https://stackoverflow.com/questions/4978323/how-to-calculate-distance-between-two-rectangles-context-a-game-in-lua
     */
    template <typename T>
    inline double distance(
        const geometry::Rectangle<T>& rect1,
        const geometry::Rectangle<T>& rect2
    ) {
        bool left   = rect2.max().x() < rect1.min().x();
        bool right  = rect1.max().x() < rect2.min().x();
        bool bottom = rect2.max().y() < rect1.min().y();
        bool top    = rect1.max().y() < rect2.min().y();

        // 2D-Cases
        if (left && top)
        {
            return distance<T>(
                { rect1.min().x(), rect1.max().y() },
                { rect2.max().x(), rect2.min().y() }
            );
        }
        else if (left && bottom)
        {
            return distance(rect1.min(), rect2.max());
        }
        else if (right && top)
        {
            return distance(rect1.max(), rect2.min());
        }
        else if (right && bottom)
        {
            return distance<T>(
                { rect1.max().x(), rect1.min().y() },
                { rect2.min().x(), rect2.max().y() }
            );
        }

        // 1D-Cases
        if (left)
        {
            return rect1.min().x() - rect2.max().x(); 
        }
        else if (right)
        {
            return rect2.min().x() - rect1.max().x();
        }
        else if (top)
        {
            return rect2.min().y() - rect1.max().y();
        }
        else if (bottom)
        {
            return rect1.min().y() - rect2.max().y();
        }         

        // Rectangles intersect
        return T(0);
    }

}
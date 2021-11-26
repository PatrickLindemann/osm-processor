#pragma once

#include <vector>

#include "model/geometry/point.hpp"
#include "model/geometry/segment.hpp"
#include "model/geometry/rectangle.hpp"
#include "model/geometry/ring.hpp"
#include "model/geometry/polygon.hpp"

#include "functions/envelope.hpp"
#include "functions/detail/shamos_hoey.hpp"

using namespace model::geometry;

namespace functions
{

    /**
     * Check if a point lies on a specified segment.
     * 
     * @param point   The point
     * @param segment The segment
     * @returns       True If the point lies on the
     *                segment.
     * 
     * Time complexity: Constant
     */
    template <typename T>
    inline bool point_in_segment(const Point<T>& point, const Segment<T>& segment)
    {
        const Point<T>& first = segment.first();
        const Point<T>& last = segment.last();
        return (point.x() <= std::max(first.x(), last.x())
             && point.x() >= std::min(first.x(), last.x())
             && point.y() <= std::max(first.y(), last.y())
             && point.y() >= std::min(first.y(), last.y()));
    }

    /**
     * Check if a point is inside of a rectangle.
     * 
     * @param point     The point
     * @param rectangle The rectangle
     * @returns         True if the point is inside
     *                  or on a segment of the rectangle
     *
     * Time complexity: Constant
     */
    template <typename T>
    inline bool point_in_rectangle(const Point<T>& point, const Rectangle<T>& rectangle)
    {
        return point.x() >= rectangle.min().x()
            && point.x() <= rectangle.max().x()
            && point.y() >= rectangle.min().y()
            && point.y() <= rectangle.max().y();
    }

    /**
     * Check if a rectangle is inside of another rectangle.
     *
     * @param rect1 The first rectangle
     * @param rect2 The second rectangle
     * @returns     True if the first rectangle is inside of
     *              in the second rectangle
     *
     * Time complexity: Constant
     */
    template <typename T>
    inline bool rectangle_in_rectangle(const Rectangle<T>& rect1, const Rectangle<T>& rect2)
    {
        return rect1.min().x() >= rect2.min().x()
            && rect1.min().y() >= rect2.min().y()
            && rect1.max().x() <= rect2.max().x()
            && rect1.max().y() <= rect2.max().y();
    }

    /**
     * Check if two segments intersect.
     * For more details on the formula, refer to
     * https://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect
     *
     * @param segment1 The first segment.
     * @param segment2 The second segment.
     * @returns true If the segments intersect.
     *
     * Time complexity: Constant
     */
    template <typename T>
    inline bool segments_intersect(const Segment<T>& s1, const Segment<T>& s2)
    {
        // Check if segments are the same
        if (s1 == s2)
        {
            return false;
        }
        const Point<T>& p0 = s1.first();
        const Point<T>& p1 = s1.last();
        const Point<T>& q0 = s2.first();
        const Point<T>& q1 = s2.last();

        // Calculate the determinant of the two segments
        Point<T> p10 = p1 - p0;
        Point<T> q10 = q1 - q0;
        double d = dot(p10, q10);

        // Check if the segments are collinear (lie on the same line)
        if (d == 0)
        {
            // Segments are collinear, check if one of the endpoints lies
            // between the endpoint of the segment
            if (dot(p10, q0 - p0) == 0)
            {
                return point_in_segment(p0, s2)
                    || point_in_segment(p1, s2)
                    || point_in_segment(q0, s1)
                    || point_in_segment(q1, s1);
            }
            return false;
        }

        // Check if the segments touch at an endpoint
        if (p0 == q0 || p0 == q1 || p1 == q0 || p1 == q1)
        {
            return false;
        }

        // Check for a point intersection
        double s = (-p10.y() * (p0.x() - q0.x()) + p10.x() * (p0.y() - q0.y()))
            / (-q10.x() * p10.y() + p10.x() * q10.y());
        double t = (q10.x() * (p0.y() - q0.y()) - q10.y() * (p0.x() - q0.x()))
            / (-q10.x() * p10.y() + p10.x() * q10.y());

        return (s >= 0 && s <= 1 && t >= 0 && t <= 1);
    }

    /**
     * Check if a point is inside of a ring using
     * the ray-casting algorithm (also knowsn as even-odd
     * rule algorithm). For more information, refer to
     * https://en.wikipedia.org/wiki/Point_in_polygon and
     * https://www.codeproject.com/Tips/84226/Is-a-Point-inside-a-Polygon
     * 
     * @param point The point
     * @param ring  The ring
     * @returns     True if the point is inside or on a segment of the ring
     * 
     * Time complexity: Linear
     */
    template <typename T>
    inline int point_in_ring(const Point<T>& point, const Ring<T>& ring)
    {
        int intersections;
        for (std::size_t i = 0; i < ring.size() - 1; i++)
        {   
            const Point<T>& first = ring.at(i);
            const Point<T>& last = ring.at(i + 1);
            // Check if point is in y-range of the ring segment (i, j)
            if (first.y() > point.y() != last.y() > point.y())
            {
                // Check for intersections
                if (point.x() < (last.x() - first.x()) * (point.y() - first.y()) / (last.y() - first.y()) + first.x())
                {
                    intersections++;
                }
            }
        }
        // Check if the point lies on the ring. If that is the case, return 0.
        if (intersections == 0)
        {
            return 0;
        }
        // If the number of intersections is odd, the return will be
        // -1 (not inside), if it is even, the result will be
        // 1 (inside)
        return (intersections % 2 == 0 ? 1 : -1);
    }

    /**
     * Check if a ring is fully contained inside of another ring.
     * 
     * @param ring1 The first ring
     * @param ring2 The second ring
     * @returns     True if the first ring is inside of the second ring
     * 
     * Time complexity: Log-Linear (Average-Case), Quadratic (Worst-Case)
     */
    template <typename T>
    inline bool ring_in_ring(const Ring<T>& ring1, const Ring<T>& ring2)
    {
        // Compare bounding boxes first
        Rectangle<T> bounds1 = functions::envelope(ring1);
        Rectangle<T> bounds2 = functions::envelope(ring2);
        if (!rectangle_in_rectangle(bounds1, bounds2))
        {
            return false;
        }

        // Check if a point from ring 1 is contained inside of ring 2
        for (const Point<T>& p : ring1)
        {
            bool b = point_in_ring(p, ring2);
            if (b < 0)
            {
                return false;
            }
            else if (b == 0)
            {
                continue;
            }

            for (std::size_t i = 0; i < ring1.size() - 1; i++)
            {
                const geometry::Segment<T> s1{ ring1.at(i), ring1.at(i + 1) };
                for (std::size_t j = i; j < ring2.size() - 1; j++)
                {
                    const geometry::Segment<T> s2{ ring2.at(j), ring2.at(j + 1) };
                    if (segments_intersect(s1, s2))
                    {
                        return false;
                    }

                }
            }
            return true;

            //// Found a point of ring 1 that is contained within ring 2
            //// Collect all segments of ring 1 and 2
            //std::vector<Segment<T>> segments{};
            //for (std::size_t i = 0; i < ring1.size() - 1; i++)
            //{
            //    segments.push_back(Segment<T>{ ring1.at(i), ring1.at(i + 1) });
            //}
            //for (std::size_t i = 0; i < ring2.size() - 1; i++)
            //{
            //    segments.push_back(Segment<T>{ ring2.at(i), ring2.at(i + 1) });
            //}

            //// Check for intersections
            //return !functions::detail::shamos_hoey(segments);
        }

        // Rings are the same
        return true;
    }
    
    template <typename T>
    inline bool polygon_in_polygon(const Polygon<T>& poly1, const Polygon<T>& poly2)
    {
        // Check outer rings first
        if (ring_in_ring(poly1.outer(), poly2.outer()))
        {
            // Verify that polygon 1 is not contained within an inner ring of
            // polygon 2
            for (const Ring<T>& inner : poly2.inners())
            {
                if (ring_in_ring(poly1.outer(), inner))
                {
                    return false;
                }
            }
            // Polygon 1 is inside of polygon 2
            return true;
        }
        return false;
    }

}
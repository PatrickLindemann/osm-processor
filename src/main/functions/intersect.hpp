#pragma once

#include <algorithm>
#include <iterator>
#include <queue>
#include <set>

#include "functions/util.hpp"
#include "functions/detail/shamos_hoey.hpp"
#include "model/geometry/line.hpp"
#include "model/geometry/point.hpp"
#include "model/geometry/ring.hpp"
#include "model/geometry/segment.hpp"
#include "model/geometry/rectangle.hpp"
#include "model/geometry/polygon.hpp"

using namespace model;

namespace functions
{

    namespace detail
    {

        /**
         * 
         */
        template <typename T>
        bool compare_xy(const geometry::Point<T>& p1, const geometry::Point<T>& p2)
        {
            // Test the x coordinate first
            if (p1.x() < p2.x()) return true;
            if (p1.x() > p2.x()) return false;
            // Test the y coordinate
            if (p1.y() < p2.y()) return true;
            // if (p1.y() > p2.y()) return false;
            // Points are the same
            return false;
        }

        template <typename T>
        std::vector<geometry::Point<T>> difference(
            std::vector<geometry::Point<T>> points1,
            std::vector<geometry::Point<T>> points2
        ) {
            std::vector<geometry::Point<T>> diff;
            std::sort(points1.begin(), points1.end(), compare_lt<T>);
            std::sort(points2.begin(), points2.end(), compare_xy<T>);
            std::set_difference(
                points1.begin(),
                points1.end(),
                points2.begin(),
                points2.end(),
                std::back_inserter(diff),
                compare_xy<T>
            );
            return diff;
        }

    }

     /**
     * Check if a point lies on a segment specified by
     * two points.
     * 
     * @param point The point
     * @param first The first point of the segment
     * @param last  The last point of the segment
     * @returns     True If the point lies on the segment
     * 
     * Time complexity: Constant
     */
    template <typename T>
    inline bool point_in_segment(
        const geometry::Point<T>& point,
        const geometry::Point<T>& first,
        const geometry::Point<T>& last
    ) {
        return (point.x() <= std::max(first.x, last.x)
             && point.x() >= std::min(first.x, last.x)
             && point.y <= std::max(first.y, last.y)
             && point.y >= std::min(first.y, last.y));
    }

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
    inline bool point_in_segment(
        const geometry::Point<T>& point,
        const geometry::Segment<T>& segment
    ) {
        const geometry::Point<T>& first = segment.first();
        const geometry::Point<T>& last = segment.last();
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
    inline bool point_in_rectangle(
        const geometry::Point<T>& point,
        const geometry::Rectangle<T>& rectangle
    ) {
        return point.x() >= rectangle.min().x()
            && point.x() <= rectangle.max().x()
            && point.y() >= rectangle.min().y()
            && point.y() <= rectangle.max().y();
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
    inline bool segments_intersect(
        const geometry::Segment<T>& segment1,
        const geometry::Segment<T>& segment2
    ) {
        // Check if segments are the same
        if (segment1 == segment2)
        {
            return false;
        }
        const geometry::Point<T>& p0 = segment1.first();
        const geometry::Point<T>& p1 = segment1.last();
        const geometry::Point<T>& q0 = segment2.first();
        const geometry::Point<T>& q1 = segment2.last();

        // Calculate the determinant of the two segments
        geometry::Point<T> p10 = p1 - p0;
        geometry::Point<T> q10 = q1 - q0;
        double d = dot(p10, q10);

        // Check if the segments are collinear (lie on the same line)
        if (d == 0)
        {
            // Segments are collinear, check if one of the endpoints lies
            // between the endpoint of the segment
            if (dot(p10, q0 - p0) == 0)
            {
                return point_in_segment(p0, segment2)
                    || point_in_segment(p1, segment2)
                    || point_in_segment(q0, segment1)
                    || point_in_segment(q1, segment1);
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
        double t =  (q10.x() * (p0.y() - q0.y()) - q10.y() * (p0.x() - q0.x()))
                  / (-q10.x() * p10.y() + p10.x() * q10.y());

        return (s >= 0 && s <= 1 && t >= 0 && t <= 1);
    }

    /*
    *
    */
    template <typename T>
    bool segments_intersect(const std::vector<geometry::Segment<T>>& segments)
    {
        for (std::size_t i = 0; i < segments.size() - 1; i++)
        {
            const geometry::Segment<T>& s1 = segments.at(i);
            for (std::size_t j = i; j < segments.size(); j++)
            {
                const geometry::Segment<T>& s2 = segments.at(j);
                if (segments_intersect(s1, s2))
                {
                    return true;
                }
            }
        }
        return false;
    }

    /**
     * Check if a point is inside of a ring using
     * the ray-casting algorithm (also knowsn as even-odd
     * rule algorithm). For more information, refer to
     * https://en.wikipedia.org/wiki/Point_in_polygon and
     * https://www.codeproject.com/Tips/84226/Is-a-geometry::Point-inside-a-Polygon
     * 
     * @param point The point
     * @param ring  The ring
     * @returns     True if the point is inside or on
     *              a segment of the ring
     * 
     * Time complexity: Linear
     */
    template <typename T>
    inline int point_in_ring(
        const geometry::Point<T>& point,
        const geometry::Ring<T>& ring
    ) {
        int intersections;
        for (std::size_t i = 0; i < ring.size() - 1; i++)
        {   
            const geometry::Point<T>& first = ring.at(i);
            const geometry::Point<T>& last = ring.at(i + 1);
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
     * Check if a rectangle is inside of another
     * rectangle.
     * 
     * @param inner The smaller rectangle
     * @param outer The larger rectangle
     * @returns     True if the first rectangle is inside of
     *              in the second rectangle
     * 
     * Time complexity: Constant
     */
    template <typename T>
    inline bool rectangle_in_rectangle(
        const geometry::Rectangle<T>& inner,
        const geometry::Rectangle<T>& outer
    ) {
        return inner.min().x() >= outer.min().x()
            && inner.min().y() >= outer.min().y()
            && inner.max().x() <= outer.max().x()
            && inner.max().y() <= outer.max().y();
    }
    
    /**
     * 
     */
    template <typename T>
    inline bool ring_self_intersects(
        const geometry::Ring<T>& ring
    ) {
        // Convert ring to segment list
        std::vector<geometry::Segment<T>> segments;
        for (std::size_t i = 0; i < ring.size() - 1; i++)
        {
            segments.push_back({ ring.at(i), ring.at(i + 1) });
        }
        // Check any of the segment pairs intersect and return
        // the result
        // return detail::intersects_shamos_hoey(segments);
        return segments_intersect(segments);
    }

    /**
     * Check if a ring is fully inside of another ring.
     * Note: The rings need to be non-self-intersecting
     * for this algorithm to work.
     * 
     * @param inner The presumed inner ring
     * @param outer The outer ring
     * @returns     True if the first ring is inside of
     *              the second ring.
     * 
     * Time complexity: Quadratic
     */
    template <typename T>
    inline bool ring_in_ring(
        const geometry::Ring<T>& inner,
        const geometry::Ring<T>& outer
    ) {
        if (inner.empty())
        {
            return false;
        }
        // Find points in the inner ring that are not part
        // of the outer ring
        std::vector<geometry::Point<T>> diff = detail::difference(inner, outer);
        if (diff.size() == 0)
        {
            // The rings are the same
            return true;
        }
        // Try to find a point from the inner ring that is
        // does not on lie on the outer ring segment and assert
        // that it is inside
        for (const geometry::Point<T>& p : diff)
        {
            int c = point_in_ring(p, outer);
            if (c < 0) return false; // Point is outside
            else if (c > 0) break; // Point is inside
        }
        // Check if there are any intersections between any
        // of the inner and outer segments
        for (std::size_t i = 0; i < inner.size() - 1; i++)
        {
            const geometry::Segment<T> s1{ inner.at(i), inner.at(i + 1) };
            for (std::size_t j = 0; j < outer.size() - 1; j++)
            {
                const geometry::Segment<T> s2{ outer.at(j), outer.at(j + 1) };
                if (segments_intersect(s1, s2))
                {
                    return false;
                }
            }
        }
        return true;
    }

}
#pragma once

#include <algorithm>
#include <queue>
#include <set>

#include "functions/util.hpp"
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

        template <typename T>
        struct Event
        {
            int edge;
            int type;
            geometry::Point<T>& point;
        };

        template <typename T>
        int compare(const Event<T>& e1, const Event<T>& e2)
        {
            const geometry::Point<T>& p1 = e1.point;
            const geometry::Point<T>& p2 = e2.point;
            // Sort by x coordinate first
            if (p1.x() > p2.x()) return 1;
            if (p1.x() < p2.x()) return -1;
            // Sort by y coordinate
            if (p1.y() > p2.y()) return 1;
            if (p1.y() < p2.y()) return -1;
            // Points are the same
            return 0;
        }

        template <typename T>
        struct SLSegment
        {
            int edge;
            geometry::Point<T> left;
            geometry::Point<T> right;
            SLSegment<T>* above;
            SLSegment<T>* below;
        };

        template <typename T>
        class SweepLine
        {

            const geometry::Line<T>& m_line;
            const size_t m_size;
            std::set<geometry::Segment<T>> m_segments;

            float is_left(
                const geometry::Point<T>& point,
                const SLSegment<T>& segment
            ) {
                const geometry::Point<T>& sl = segment.left;
                const geometry::Point<T>& sr = segment.right;
                return (sr.x() - sl.x()) * (point.y() - sl.y())
                     - (sr.y() - sl.y()) * (point.x() - sl.x());
            }

        public:

            SweepLine<T>(const geometry::Line<T>& line) : m_line(line), m_size(line.size()) {};

            inline SLSegment<T>& add(Event<T> e)
            {

            }

            inline SLSegment<T>& find(Event<T> e)
            {

            }

            inline bool intersect(const SLSegment<T>& s1, const SLSegment<T>& s2)
            {
                // Check if the edges are consecutive in the line
                const int& e1 = s1.edge;
                const int& e2 = s2.edge;
                if ((e1 + 1) % m_size == e2 || (e2 + 1) % m_size == e1)
                {
                    return false;
                }
                // Test for existence of an intersection point
                float l_sign = is_left(s1.left, s2);
                float r_sign = is_left(s1.right, s2);
                if (l_sign * r_sign > 0)
                {
                    // Endpoints of s1 have the same sign relative
                    // to s2
                    return false;
                }
                l_sign = is_left(s2.left, s1);
                r_sign = is_left(s2.right, s1);
                if (l_sign * r_sign > 0)
                {
                    // Endpoints of s2 have the same sign relative
                    // to s1
                    return false;
                }
                // Found intersection
                return true;
            }

            inline void remove(SLSegment<T>* s)
            {

            }

        };


        /**
         * 
         */
        template <typename T>
        bool shamos_hoey(std::vector<geometry::Segment<T>>& segments)
        {
            return false;
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
     * https://www.geeksforgeeks.org/check-if-two-given-line-segments-intersect/.
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
        // Calculate the dot product of the segments
        const geometry::Point<T> pd = p1 - p0;
        const T d = functions::dot(pd, q1 - q0);
        // Check if segments are collinear
        if (d != 0)
        {
            // Segments are not collinear, check if
            // they touch at an end point
            if (p0 == q0 || p0 == q1 || p1 == q0 || p1 == q1)
            {
                return false;
            }
            // Check for point intersection
            const T na = (q1.x() - q0.x()) * (p0.y() - q0.y())
                             - (q1.y() - q0.y()) * (p0.x() - q0.x());
            const T nb = (p1.x() - p0.x()) * (p0.y() - q0.y())
                             - (p1.y() - p0.y()) * (p0.x() - q0.x());
            return (d > 0 && na >= 0 && na <= d && nb >= 0 && nb <= d)
                || (d < 0 && na <= 0 && na >= d && nb <= 0 && nb >= d);
        }
        // Segments are collinear, check if they lie on
        // the same line
        if (dot(pd, q0 - p0) == 0)
        {
            return point_in_segment(p0, segment2)
                || point_in_segment(p1, segment2)
                || point_in_segment(q0, segment1)
                || point_in_segment(q1, segment1);
        }
        return false;
    }

    /*
    *
    */
    template <typename T>
    bool segments_intersect(const std::vector<geometry::Segment<T>>& segments)
    {
        for (size_t i = 0; i < segments.size() - 1; i++)
        {
            const geometry::Segment<T>& s1 = segments.at(i);
            for (size_t j = i; j < segments.size(); j++)
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
    inline bool point_in_ring(
        const geometry::Point<T>& point,
        const geometry::Ring<T>& ring
    ) {
        bool c;
        for (size_t i = 0, j = ring.size() - 1; i < ring.size(); j = i++)
        {   
            const geometry::Point<T>& first = ring.at(i);
            const geometry::Point<T>& last = ring.at(j);
            // Check if point is in y-range of the ring segment (i, j)
            if (first.y() > point.y() != last.y() > point.y())
            {
                // Check for intersections
                if (point.x() < (last.x() - first.x()) * (point.y() - first.y()) / (last.y() - first.y()) + first.x())
                {
                    c = !c;
                }
            }
        }
        // If the number of intersections is odd, the return will be
        // false (not inside), if it is even, the result will be
        // true (inside)
        return c;
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
        for (size_t i = 0, j = ring.size() - 1; i < ring.size(); j = i++)
        {
            segments.push_back({ ring.at(j), ring.at(i) });
        }
        // Check any of the segment pairs intersect and return
        // the result
        return !segments_intersect(segments);
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
        // Check if a random point of the inner ring is
        // inside of the outer ring
        if (point_in_ring(inner.at(0), outer))
        {
            return false;
        }
        // Check if there are any intersections between any
        // of the inner and outer segments
        for (size_t i1 = 0, j1 = inner.size() - 1; i1 < inner.size() - 1; j1 = i1++)
        {
            const geometry::Segment<T> s1{ inner.at(j1), inner.at(i1) };
            for (size_t i2 = 0, j2 = outer.size() - 1; i2 < outer.size(); j2 = i2++)
            {
                const geometry::Segment<T> s2{ outer.at(j2), inner.at(i2) };
                if (segments_intersect(s1, s2))
                {
                    return false;
                }
            }
        }
        return true;
    }

}
#pragma once

#include "model/geometry/point.hpp"
#include "model/geometry/segment.hpp"
#include "model/geometry/rectangle.hpp"
#include "model/geometry/polygon.hpp"

namespace functions
{

    using namespace model;

     /**
     * Check if a point lies on a segment between two specifed
     * points.
     * 
     * @param point The point.
     * @param first The first point of the segment.
     * @param last The last point of the segment.
     * @returns true If the point lies on the segment.
     */
    template <typename T>
    inline bool point_in_segment(
        const geometry::Point<T>& point,
        const geometry::Point<T>& first,
        const geometry::Point<T>& last
    ) {
        return (point.x <= std::max(first.x, last.x)
             && point.x >= std::min(first.x, last.x)
             && point.y <= std::max(first.y, last.y)
             && point.y >= std::min(first.y, last.y));
    }
    
    template <typename T>
    inline bool point_in_polygon(
        const geometry::Point<T>& point,
        const geometry::Polygon<T>& polygon
    ) {
        
    }

    /**
     * Check if a point lies on a specified segment.
     * 
     * @param point The point.
     * @param segment The segment.
     * @returns true If the point lies on the segment.
     */
    template <typename T>
    inline bool point_in_segment(
        const geometry::Point<T>& point,
        const geometry::Segment<T>& segment
    ) {
        const geometry::Point<T>& first = segment.first();
        const geometry::Point<T>& last = segment.last();
        return (point.x <= std::max(first.x, last.x)
             && point.x >= std::min(first.x, last.x)
             && point.y <= std::max(first.y, last.y)
             && point.y >= std::min(first.y, last.y));
    }

    /**
     * Check if a rectangle is completly contained in another
     * rectangle.
     * 
     * @param small_rect The smaller rectangle
     * @param larger_rect The larger rectangle
     * @returns True if the first rectangle is contained in the second.
     */
    template <typename T>
    inline bool rectangle_in_rectangle(
        const geometry::Rectangle<T>& small_rect,
        const geometry::Rectangle<T>& larger_rect
    ) {
        return small_rect.min.x >= larger_rect.min.x
            && small_rect.min.y >= larger_rect.min.y
            && small_rect.max.x <= larger_rect.max.x
            && small_rect.max.y <= larger_rect.max.y;
    }

    /**
     * Check if two segments intersect.
     * Refer to https://www.geeksforgeeks.org/check-if-two-given-line-segments-intersect/
     * for details of the formula.
     * 
     * @param segment1 The first segment.
     * @param segment2 The second segment.
     * @returns true If the segments intersect.
     */
    template <typename T>
    inline bool intersect(
        const geometry::Segment<T> segment1,
        const geometry::Segment<T> segment2
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
        const T d = dot(pd, q1 - q0);
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
            const T na = (q1.x - q0.x) * (p0.y - q0.y)
                             - (q1.y - q0.y) * (p0.x - q0.x);
            const T nb = (p1.x - p0.x) * (p0.y - q0.y)
                             - (p1.y - p0.y) * (p0.x - q0.x);
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

}
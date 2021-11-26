#pragma once

#include <utility>

#include "model/geometry/point.hpp"

namespace model
{

    namespace geometry
    {
        
        /**
         * 
         */
        template <typename T>
        class Segment
        {

            /* Members */

            std::pair<Point<T>, Point<T>> points;

        public:

            /* Constructors */

            Segment() {}
            Segment(Point<T> first, Point<T> last) : points(first, last) {}
            Segment(std::pair<T, T> segment) : points(segment) {}

            /* Accessors */

            Point<T>& first()
            {
                return points.first;
            }

            const Point<T>& first() const
            {
                return points.first;
            }

            Point<T>& last()
            {
                return points.second;
            }

            const Point<T>& last() const
            {
                return points.second;
            }

            /* Operators */

            bool operator==(const Segment<T>& other) const
            {
                return (points.first == other.first() && points.second == other.last())
                    || (points.first == other.last() && points.second == other.first());
            }

            bool operator!=(const Segment<T>& other) const
            {
                return (points.first != other.first() || points.second != other.last())
                    && (points.first != other.last() || points.second != other.first());
            }

        };

    }

}
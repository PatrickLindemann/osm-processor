#pragma once

#include <array>

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
        public:

            /* Types */

            using point_type     = Point<T>;
            using container_type = std::array<point_type, 2>;
            using iterator       = typename container_type::iterator;
            using const_iterator = typename container_type::const_iterator;

            /* Members */

            container_type points;

            /* Constructor */

            Segment(point_type first, point_type last) : points({ first, last }) {};

            /* Methods */

            const bool valid() const
            {
                return points.size() == 2;
            } 

            const bool empty() const
            {
                return points.empty();
            }

            size_t size() const
            {
                return points.size();
            }

            const point_type& first() const
            {
                return points[0];
            }

            const point_type& last() const
            {
                return points[1];
            }

            point_type& at(size_t index)
            {
                return points.at(index);
            }
            
            const point_type& at(size_t index) const
            {
                return points.at(index);
            }

            point_type& operator[](const size_t index)
            {
                return points[index];
            }

            const point_type& operator[](const size_t index) const
            {
                return points[index];
            }

            bool operator==(const Segment<T>& other) const
            {
                return (points[0] == other.first() && points[1] == other.last())
                    || (points[0] == other.last() && points[1] == other.first());
            }

            bool operator!=(const Segment<T>& other) const
            {
                return (points[0] != other.first() || points[1] != other.last())
                    && (points[0] != other.last() || points[1] != other.first());
            }

        };

    }

}
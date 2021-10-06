#pragma once

#include <vector>

#include "model/geometry/point.hpp"

namespace model
{

    namespace geometry
    {

        template <typename T>
        class Line
        {
        public:

            /* Types */

            using point_type     = Point<T>;
            using container_type = std::vector<point_type>;
            using iterator       = typename container_type::iterator;
            using const_iterator = typename container_type::const_iterator;

            /* Members */

            container_type points;

            /* Constructors */

            Line() {};
            Line(container_type& points) : points(points) {};
            Line(std::initializer_list<point_type>& points) : points(points) {};

            /* Methods */

            const bool valid() const
            {
                return points.size() >= 2;
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
                return points[points.size() - 1];
            }

            point_type& at(size_t index)
            {
                return points.at[index];
            }
            
            const point_type& at(size_t index) const
            {
                return points.at[index];
            }

            point_type& operator[](const size_t index)
            {
                return points[index];
            }

            const point_type& operator[](const size_t index) const
            {
                return points[index];
            }
            
            iterator begin() noexcept
            {
                return points.begin();
            }

            const_iterator begin() const noexcept
            {
                return points.cbegin();
            }

            iterator end() noexcept
            {
                return points.end();
            }

            const_iterator end() const noexcept
            {
                return points.cend();
            }

        };

    }

}
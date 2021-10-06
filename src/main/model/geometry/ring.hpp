#pragma once

#include <vector>

#include "model/geometry/point.hpp"

namespace model
{

    namespace geometry
    {

        template <typename T>
        class Ring
        {
        public:

            /* Types */

            using point_type             = Point<T>;
            using container_type         = std::vector<point_type>;
            using iterator               = typename container_type::iterator;
            using const_iterator         = typename container_type::const_iterator;
            using reverse_iterator       = typename container_type::reverse_iterator;
            using const_reverse_iterator = typename container_type::const_reverse_iterator;

            /* Members */

            container_type points;

            /* Constructors */

            Ring() {};
        
            Ring(container_type& points) : points(points)
            {
                if (points.front() != points.back())
                {
                    points.push_back(points.back());
                }
            };

            /* Methods */

            const bool valid() const
            {
                return points.size() >= 3 && points.front() == points.back();
            }

            void close()
            {
                if (points.front() != points.back())
                {
                    points.push_back(points.front());
                }
            }

            const bool empty() const
            {
                return points.empty();
            }

            size_t size() const
            {
                return points.size();
            }

            void push_back(point_type point)
            {
                points.push_back(point);
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
            
            reverse_iterator rbegin() noexcept
            {
                return points.rbegin();
            }

            const_reverse_iterator rbegin() const noexcept
            {
                return points.crbegin();
            }

            reverse_iterator rend() noexcept
            {
                return points.rend();
            }

            const_reverse_iterator rend() const noexcept
            {
                return points.crend();
            }

        };

    }

}
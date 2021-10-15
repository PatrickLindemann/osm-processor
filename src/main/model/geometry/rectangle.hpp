#pragma once

#include "model/geometry/point.hpp"

namespace model
{    

    namespace geometry
    {
    
        template <typename T>
        class Rectangle
        {

            /* Members */
            
            Point<T> m_min;
            Point<T> m_max;

        public:

            /* Constructors */

            Rectangle() : m_min(0, 0), m_max(0, 0) {};
            Rectangle(T min_x, T min_y, T max_x, T max_y) : m_min(min_x, min_y), m_max(max_x, max_y) {};
            Rectangle(Point<T> min, Point<T> max) : m_min(min), m_max(max) {};

            /* Accessors */

            Point<T>& min()
            {
                return m_min;
            }

            const Point<T>& min() const
            {
                return m_min;
            }

            Point<T>& max()
            {
                return m_max;
            }

            const Point<T>& max() const
            {
                return m_max;
            }

            /* Methods */

            bool valid() const
            {
                return m_min.x() <= m_max.x() && m_min.y() <= m_max.y(); 
            }
    
            double width() const
            {
                return m_max.x() -  m_min.x();
            }

            double height() const
            {
                return m_max.y() - m_min.y();
            }

            Point<T> top_left() const
            {
                return m_min;
            }

            Point<T> top_right() const
            {
                return Point<T>{ m_max.x(), m_min.y() };
            }

            Point<T> bottom_left() const
            {
                return Point<T>{ m_min.x(), m_max.y() };
            }

            Point<T> bottom_right() const
            {
                return m_max;
            }

        };

    }

}
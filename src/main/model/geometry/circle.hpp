#pragma once

#include "model/geometry/point.hpp"

namespace model
{

    namespace geometry
    {
        
        /**
         * 
         */
        template <typename T>
        class Circle
        {
            /* Members */

            Point<T> m_center;
            T m_radius;

    	public:

            /* Constructors */

            Circle(Point<T> center, T radius) : m_center(center), m_radius(radius) {};

            /* Accessors */

            Point<T>& center()
            {
                return m_center;
            }

            const Point<T>& center() const
            {
                return m_center;
            }

            T& radius()
            {
                return m_radius;
            }

            const T& radius() const
            {
                return m_radius;
            }

            /* Methods */

            bool valid() const
            {
                return m_radius > 0;
            }

            T diameter() const
            {
                return m_radius * 2;
            }

        };

    }

}
#pragma once

#include <vector>

#include "model/geometry/point.hpp"
#include "model/geometry/ring.hpp"

namespace model
{

    namespace geometry
    {

        template <typename T>
        class Polygon
        {
            /* Members */

            Ring<T> m_outer;
            std::vector<Ring<T>> m_inners;

        public:

            /* Constructors */

            Polygon() {};
            Polygon(Ring<T> outer) : m_outer(outer) {};
            Polygon(Ring<T> outer, std::vector<Ring<T>> inners) : m_outer(outer), m_inners(inners) {};

            /* Accessors */

            Ring<T>& outer()
            {
                return m_outer;
            }

            const Ring<T>& outer() const
            {
                return m_outer;
            }

            Ring<T>& inners()
            {
                return m_inners;
            }

            const Ring<T>& inners() const
            {
                return m_inners;
            }

        };

    }

}
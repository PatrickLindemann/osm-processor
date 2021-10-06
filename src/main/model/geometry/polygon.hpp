#pragma once

#include <vector>

#include "model/geometry/point.hpp"
#include "model/geometry/ring.hpp"

namespace model
{

    namespace geometry
    {

        /**
         * 
         */
        template <typename T>
        class Polygon
        {
        public:

            /* Types */

            using point_type   = Point<T>;
            using ring_type    = Ring<T>;

            /* Members */

            ring_type outer;
            std::vector<ring_type> inners;

            /* Constructors */

            Polygon() {};
            Polygon(ring_type& outer) : outer(outer) {};
            Polygon(ring_type& outer, std::vector<ring_type>& inners) : outer(outer), inners(inners) {};

        };

    }

}
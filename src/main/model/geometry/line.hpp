#pragma once

#include <vector>

#include "model/geometry/point.hpp"

namespace model
{

    namespace geometry
    {

        template <typename T>
        class Line : public std::vector<Point<T>>
        {
        public:

            /* Methods */

            /**
             * Check if this Line is valid, i.e. if it has
             * at least two nodes.
             */
            const bool valid() const
            {
                return this->size() >= 2;
            } 

        };

    }

}
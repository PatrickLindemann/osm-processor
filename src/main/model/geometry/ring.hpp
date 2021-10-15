#pragma once

#include <vector>

#include "model/geometry/line.hpp"

namespace model
{

    namespace geometry
    {

        template <typename T>
        class Ring : public Line<T>
        {
        public:

            /* Methods */

            /**
             * Check if this ring is valid, i.e if it has at least
             * three points.
             */
            bool valid() const
            {
                return this->size() >= 3;
            }

            /**
             * Check if this ring is closed, i.e. if the first and
             * the last point are the same.
             */
            bool is_closed() const
            {
                return this->front() == this->back();
            }

            /**
             * Close this ring if it is not closed already.
             */
            void close()
            {
                if (this->front() != this->back())
                {
                    this->push_back(this->front());
                }
            }

        };

    }

}
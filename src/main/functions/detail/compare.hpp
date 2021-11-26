#pragma once

#include "model/geometry/point.hpp"

using namespace model::geometry;

namespace functions
{

    namespace detail
    {

        /* Functions */

        /**
         * Checks if a point p is less than another point q by comparing their xy-order.
         *
         * @param   p The first point
         * @param   q The second point
         * @returns True if p has a strictly lower xy-order than q.
         */
        template <typename T>
        bool compare_lt(const Point<T>& p, const Point<T>& q)
        {
            // Compare the x coordinate first
            if (p.x() < q.x()) return true;
            if (p.x() > q.x()) return false;
            // Compare the y coordinate
            if (p.y() < q.y()) return true;
            // if (p.y() < q.y()) return false;
            // Points are the same
            return false;
        }

        /**
         * Checks if a point p is greater than another point q by comparing their xy-order.
         *
         * @param   p The first point
         * @param   q The second point
         * @returns True if p has a strictly greater xy-order than q.
         */
        template <typename T>
        bool compare_gt(const Point<T>& p, const Point<T>& q)
        {
            // Compare the x coordinate first
            if (p.x() > q.x()) return true;
            if (p.x() < q.x()) return false;
            // Compare the y coordinate
            if (p.y() > q.y()) return true;
            // if (p.y() < q.y()) return false;
            // Points are the same
            return false;
        }

    }

}
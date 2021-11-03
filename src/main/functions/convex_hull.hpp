#pragma once

#include "model/geometry/point.hpp"
#include "model/geometry/ring.hpp"

using namespace model;

namespace functions
{

    template <typename T>
    geometry::Ring<T> convex_hull(const std::vector<geometry::Point<T>>& points)
    {
        geometry::Ring<T>{};
    }

}
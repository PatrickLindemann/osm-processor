#pragma once

#include "model/geometry/point.hpp"
#include "model/geometry/rectangle.hpp"
#include "model/geometry/multipolygon.hpp"
#include "model/types.hpp"

namespace model
{

    template <typename T>
    struct Boundary
    {
        object_id_type id;
        std::string name;
        level_type level;
        geometry::MultiPolygon<T> geometry;
        geometry::Rectangle<T> bounds;
        geometry::Point<T> center;
    };

}
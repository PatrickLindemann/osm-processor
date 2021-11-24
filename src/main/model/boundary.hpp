#pragma once

#include "model/geometry/point.hpp"
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
        model::geometry::MultiPolygon<T> geometry;
        model::geometry::Point<T> center;
    };

}
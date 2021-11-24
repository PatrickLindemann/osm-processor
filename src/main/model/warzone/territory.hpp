#pragma once

#include <string>
#include <vector>

#include "model/geometry/point.hpp"
#include "model/geometry/multipolygon.hpp"

namespace model
{

    namespace warzone
    {

        template <typename T>
        struct Territory
        {
            object_id_type id;
            std::string name;
            geometry::MultiPolygon<T> geometry;
            geometry::Point<T> center;
            std::vector<object_id_type> neighbors;
        };
    
    }

}
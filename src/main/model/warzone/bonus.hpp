#pragma once

#include <string>
#include <vector>

#include "model/types.hpp"
#include "model/geometry/point.hpp"
#include "model/geometry/multipolygon.hpp"

namespace model
{

    namespace warzone
    {

        template <typename T>
        struct Bonus
        {
            object_id_type id;
            std::string name;
            geometry::MultiPolygon<T> geometry;
            geometry::Point<T> center;
            std::vector<object_id_type> children;
            army_type armies;
            std::string color;
        };
    
        template <typename T>
        struct SuperBonus : public Bonus<T> {};

    }

}
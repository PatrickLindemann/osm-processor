#pragma once

#include <vector>

#include "model/geometry/point.hpp"
#include "model/geometry/polygon.hpp"

namespace model
{

    namespace geometry
    {
        
        /**
         * 
         */
        template <typename T>
        class MultiPolygon
        {
        public:

            /* Types */

            using point_type     = Point<T>;
            using polygon_type   = Polygon<T>;
            using container_type = std::vector<polygon_type>;

            /* Members */

            container_type polygons;

            /* Constructors */

            MultiPolygon() {};
            MultiPolygon(container_type& polygons) : polygons(polygons) {};
            MultiPolygon(std::initializer_list<polygon_type>& polygons) : polygons(polygons) {};

            /* Misc */

            const bool is_polygon() const
            {
                return polygons.size() == 1;
            }

        };

    }

}
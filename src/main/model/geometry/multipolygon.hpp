#pragma once

#include <vector>

#include "model/geometry/polygon.hpp"

namespace model
{

    namespace geometry
    {
        
        template <typename T>
        class MultiPolygon
        {
            /* Members */

            std::vector<Polygon<T>> m_polygons;

        public:

            /* Constructors */

            MultiPolygon() {};
            MultiPolygon(std::vector<Polygon<T>>& polygons) : m_polygons(polygons) {};

            /* Accessors */

            std::vector<Polygon<T>>& polygons()
            {
                return m_polygons;
            }

            const std::vector<Polygon<T>>& polygons() const
            {
                return m_polygons;
            }

            /* Methods */

            const bool is_polygon() const
            {
                return m_polygons.size() == 1;
            }

        };

    }

}
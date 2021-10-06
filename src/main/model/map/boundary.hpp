#pragma once

#include <string>

#include "model/geometry/point.hpp"
#include "model/geometry/multipolygon.hpp"
#include "model/geometry/rectangle.hpp"

namespace model
{

    namespace map
    {

        class Boundary
        {
        public:

            /* Classes */

            enum Type
            {
                TERRITORY, BONUS
            };

            /* Types */

            using id_type       = unsigned long;
            using point_type    = geometry::Point<double>;
            using geometry_type = geometry::MultiPolygon<double>;

        protected:

            /* Members */

            id_type m_id;
            Type m_type;
            std::string m_name;
            int m_level;
            geometry_type m_geometry;
            point_type m_center;

        public:

            /* Constructors */

            Boundary(
                id_type id,
                Type type,
                std::string name,
                int level,
                geometry_type& geometry,
                point_type& center
            ) : m_id(id), m_type(type), m_name(name), m_level(level), m_geometry(geometry), m_center(center) {};

            /* Accessors */

            const id_type id() const
            {
                return m_id;
            }

            const Type type() const
            {
                return m_type;
            };

            const std::string& name() const
            {
                return m_name;
            }

            const int level() const
            {
                return m_level;
            }

            const geometry_type& geometry() const
            {
                return m_geometry;
            }

            const point_type& center() const
            {
                return m_center;
            }

        };    

    }

}
#pragma once

#include <string>

#include "model/geometry/multipolygon.hpp"
#include "model/geometry/point.hpp"
#include "model/geometry/rectangle.hpp"
#include "model/map/entity.hpp"

namespace model
{

    namespace map
    {

        using namespace model::geometry;

        /**
         * 
         */
        class TerritoryRef : public EntityRef
        {
        public:

            /* Constructors */

            TerritoryRef(id_type ref) : EntityRef(ref) {};

        };

        class Territory : public Entity
        {

            /* Members */

            /**
             * 
             */
            std::string m_name;

            /**
             * 
             */
            Point<double> m_center;

            /**
             * 
             */
            std::vector<TerritoryRef> m_neighbors;

            /**
             * 
             */
            MultiPolygon<double> m_geometry;

        public:

            /* Constructors */

            Territory(id_type id) : Entity(id) {};

            /* Accessors */

            std::string& name()
            {
                return m_name;
            }

            const std::string& name() const
            {
                return m_name;
            }

            Point<double>& center()
            {
                return m_center;
            }

            const Point<double>& center() const
            {
                return m_center;
            }

            std::vector<TerritoryRef>& neighbors()
            {
                return m_neighbors;
            }

            const std::vector<TerritoryRef>& neighbors() const
            {
                return m_neighbors;
            }

            MultiPolygon<double>& geometry()
            {
                return m_geometry;
            }

            const MultiPolygon<double>& geometry() const
            {
                return m_geometry;
            }

        };    

    }

}
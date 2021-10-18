#pragma once

#include <string>

#include "model/geometry/multipolygon.hpp"
#include "model/geometry/point.hpp"
#include "model/geometry/rectangle.hpp"
#include "model/map/boundary.hpp"

namespace model
{

    namespace map
    {

        using namespace model::geometry;

        class TerritoryRef : public BoundaryRef {};

        class Territory : public Boundary
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
            Polygon<double> m_geometry;

        public:

            /* Constructors */

            Territory(object_id_type id) : Boundary(id) {};

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

            Polygon<double>& geometry()
            {
                return m_geometry;
            }

            const Polygon<double>& geometry() const
            {
                return m_geometry;
            }

        };    

    }

}
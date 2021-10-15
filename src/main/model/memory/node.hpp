#pragma once

#include "model/geometry/point.hpp"
#include "model/memory/entity.hpp"

namespace model
{

    namespace memory
    {

        /**
         * A Node is a native OSMObject that stores a location with
         * longtitude and latitude.
         */
        class Node : public Entity
        {

            /* Types */

            /**
             * Longtitude and latitude coordinates are floating-point
             * values. These will be stored as point with double-precision.
             */
            using point_type = geometry::Point<double>;

        protected:

            /* Members */

            point_type m_point;

        public:

            /* Constructors */

            Node(object_id_type id) : Entity(id) {};
            Node(object_id_type id, double lon, double lat) : Entity(id), m_point(lon, lat) {};
            Node(object_id_type id, geometry::Point<double> point) : Entity(id), m_point(point) {};

            /* Accessors */

            point_type& point()
            {
                return m_point;
            }

            const point_type& point() const
            {
                return m_point;
            }

            double& lon()
            {
                return m_point.x();
            }

            const double& lon() const
            {
                return m_point.x();
            }

            double& lat()
            {
                return m_point.y();
            }

            const double& lat() const
            {
                return m_point.y();
            }

        };

        class NodeRef : public EntityRef {};

    }
    
}
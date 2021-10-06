#pragma once

#include <cmath>

#include "model/memory/entity.hpp"
#include "model/geometry/point.hpp"

namespace model
{

    namespace memory
    {

        /**
         * 
         */
        class Node : public Entity
        {
        public:

            using point_type = geometry::Point<double>;

        protected:

            /* Members */

            point_type m_point;

        public:

            /* Constructors */

            /**
             * 
             */
            Node(id_type id) : Entity(id), m_point(0.0, 0.0) {};

            /**
             * 
             */
            Node(id_type id, double x, double y) : Entity(id), m_point(x, y) {};

            /**
             * 
             */
            Node(id_type id, geometry::Point<double> point) : Entity(id), m_point(point) {};
            
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
                return m_point.x;
            }

            const double& lon() const
            {
                return m_point.x;
            }

            double& lat()
            {
                return m_point.y;
            }

            const double& lat() const
            {
                return m_point.y;
            }

        };

    }
    
}
#pragma once

#include "model/memory/entity.hpp"
#include "model/geometry/point.hpp"

namespace model
{

    namespace memory
    {

        template <typename T>
        class Node : public Entity
        {
        public:

            using point_type = geometry::Point<T>;

        protected:

            point_type m_point;

        public:

            Node(id_type id) : Entity(id), m_point( T(0), T(0) ) {};
            Node(id_type id, double lon, double lat) : Entity(id), m_point(lon, lat) {};
            Node(id_type id, geometry::Point<double> point) : Entity(id), m_point(point) {};

            point_type& point()
            {
                return m_point;
            }

            const point_type& point() const
            {
                return m_point;
            }

            T& lon()
            {
                return m_point.x;
            }

            const T& lon() const
            {
                return m_point.x;
            }

            T& lat()
            {
                return m_point.y;
            }

            const T& lat() const
            {
                return m_point.y;
            }

        };

    }
    
}
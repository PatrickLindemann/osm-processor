#pragma once

#include <memory>
#include <set>
#include <string>
#include <vector>

#include "model/map/boundary.hpp"
#include "model/geometry/point.hpp"
#include "model/geometry/multipolygon.hpp"

namespace model
{

    namespace map
    {

        /**
         * 
         */
        class Map
        {
        public:

            /* Types */

            using point_type = geometry::Point<double>;

        protected:

            /* Members */

            int m_width;
            int m_height;
            std::vector<map::Boundary> m_boundaries;
            // std::set<long, long> neighbors;

        public:

            /* Constructor */

            Map(
                int width,
                int height,
                std::vector<map::Boundary> & boundaries
            ) : m_width(width), m_height(height), m_boundaries(boundaries) {};

            /* Accessors */

            const int width() const
            {
                return m_width;
            }

            const int height() const
            {
                return m_height;
            }

            const std::vector<map::Boundary> boundaries() const
            {
                return m_boundaries;
            }
            
        };

    }
    
}
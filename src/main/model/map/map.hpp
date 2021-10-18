#pragma once

#include <memory>
#include <set>
#include <string>
#include <vector>

#include "model/geometry/point.hpp"
#include "model/geometry/multipolygon.hpp"
#include "model/map/bonus.hpp"
#include "model/map/territory.hpp"
#include "model/type.hpp"

namespace model
{

    namespace map
    {
        
        /**
         * 
         */
        class Map
        {

            /* Members */

            /**
             * 
             */
            size_t m_width;

            /**
             * 
             */
            size_t m_height;

            /**
             * 
             */
            std::vector<Territory> m_territories;

            /**
             * 
             */
            std::vector<Bonus> m_bonuses;

        public:

            /* Constructors */

            Map() {};

            /* Accessors */

            size_t& width()
            {
                return m_width;
            }

            const size_t& width() const
            {
                return m_width;
            }

            size_t& height()
            {
                return m_height;
            }

            const size_t& height() const
            {
                return m_height;
            }

            std::vector<Territory>& territories()
            {
                return m_territories;
            }

            const std::vector<Territory>& territories() const
            {
                return m_territories;
            }

            std::vector<Bonus>& bonuses()
            {
                return m_bonuses;
            }

            const std::vector<Bonus>& bonuses() const
            {
                return m_bonuses;
            }
            
        };

    }
    
}
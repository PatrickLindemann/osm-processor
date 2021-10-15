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
            std::string m_name;

            /**
             * 
             */
            width_type m_width;

            /**
             * 
             */
            height_type m_height;

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

            std::string& name()
            {
                return m_name;
            }

            const std::string& name() const
            {
                return m_name;
            }

            width_type& width()
            {
                return m_width;
            }

            const width_type& width() const
            {
                return m_width;
            }

            width_type& height()
            {
                return m_height;
            }

            const width_type& height() const
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
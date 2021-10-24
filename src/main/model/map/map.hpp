#pragma once

#include <memory>
#include <set>
#include <string>
#include <vector>

#include "model/geometry/point.hpp"
#include "model/geometry/segment.hpp"
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
            size_t m_width;

            /**
             * 
             */
            size_t m_height;

            /**
             * 
             */
            std::vector<level_type> m_levels;

            /**
             * 
             */
            std::vector<Territory> m_territories;

            /**
             * 
             */
            std::vector<Bonus> m_bonuses;


            /**
             * 
             */
            std::vector<SuperBonus> m_super_bonuses;

            /**
             * 
             */
            std::vector<geometry::Segment<double>> m_connections;

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

            std::vector<level_type>& levels()
            {
                return m_levels;
            }

            const std::vector<level_type>& levels() const
            {
                return m_levels;
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
            
            std::vector<SuperBonus>& super_bonuses()
            {
                return m_super_bonuses;
            }

            const std::vector<SuperBonus>& super_bonuses() const
            {
                return m_super_bonuses;
            }

            std::vector<geometry::Segment<double>>& connections()
            {
                return m_connections;
            }

            const std::vector<geometry::Segment<double>>& connections() const
            {
                return m_connections;
            }

        };

    }
    
}
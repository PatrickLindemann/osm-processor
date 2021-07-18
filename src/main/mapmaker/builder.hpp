#ifndef MAPMAKER_BUILDER_HPP
#define MAPMAKER_BUILDER_HPP

#include <vector>

#include "mapmaker/model.hpp"

namespace mapmaker
{

    namespace builder
    {
        
        class Builder
        {
        public:

            typedef geometry::model::Point<double_t> point_type;
            typedef std::vector<model::Boundary> boundary_array_type;

        private:
            
            int32_t m_territory_level;
            int32_t m_bonus_level;
            double_t m_epsilon;
            boundary_array_type m_territories;
            boundary_array_type m_bonus_links;

        public:

            Builder() {};
            ~Builder() {};

            /* Setters */

            void set_territories(boundary_array_type& boundaries, int32_t territory_level)
            {
                this->m_territories = {};
                this->m_territory_level = territory_level;
                for (model::Boundary& boundary : boundaries)
                    if (boundary.level == territory_level)
                        this->m_territories.push_back(boundary);
            }

            void set_bonus_links(boundary_array_type& boundaries, int32_t bonus_level)
            {
                this->m_bonus_links = {};
                this->m_bonus_level = bonus_level;
                for (model::Boundary& boundary : boundaries)
                    if (boundary.level == bonus_level)
                        this->m_territories.push_back(boundary);
            }

            void set_epsilon(double_t epsilon)
            {
                this->m_epsilon = epsilon;
            }

            /* Methods */

            model::Map build() const
            {
                // TODO Compression, Relationships, center points, polygon checks, troop circle position, etc.
                return model::Map{ m_territories, m_bonus_links };
            }
            
        };

    }

}

#endif
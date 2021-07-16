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
            int32_t m_width;
            int32_t m_height;
            double_t m_epsilon;
            boundary_array_type m_boundaries;

        public:

            Builder() {};
            Builder(boundary_array_type& boundaries, int32_t territory_level)
            : m_boundaries(boundaries),
              m_territory_level(territory_level)
            {};
            Builder(boundary_array_type& boundaries, int32_t territory_level, int32_t bonus_level)
            : m_territory_level(territory_level),
              m_bonus_level(bonus_level)
            {};
            Builder(boundary_array_type& boundaries, int32_t territory_level, int32_t bonus_level, int32_t width, int32_t height) 
            : m_boundaries(boundaries),
              m_territory_level(territory_level),
              m_bonus_level(bonus_level),
              m_width(width),
              m_height(height)
            {};
            Builder(boundary_array_type& boundaries, int32_t territory_level, int32_t bonus_level, int32_t width, int32_t height, double_t epsilon) 
            : m_boundaries(boundaries),
              m_territory_level(territory_level),
              m_bonus_level(bonus_level),
              m_width(width),
              m_height(height),
              m_epsilon(epsilon)
            {};

            ~Builder() {};

            /* Accessors and Setters */

            const int32_t width() const
            {
                return m_width;
            }

            void set_width(const int32_t width)
            {
                this->m_width = width;
            }

            const int32_t height() const
            {
                return m_height;
            }

            void set_height(const int32_t height)
            {
                this->m_height = height;
            }

            /* Methods */

            model::Map build() const
            {
                return model::Map{};
            }
            
        };

    }

}

#endif
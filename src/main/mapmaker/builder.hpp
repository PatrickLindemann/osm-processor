#ifndef MAPMAKER_BUILDER_HPP
#define MAPMAKER_BUILDER_HPP

#include <vector>

#include "geometry/algorithm.hpp"
#include "geometry/model.hpp"
#include "mapmaker/model.hpp"

namespace mapmaker
{

    namespace builder
    {
        
        class Builder
        {
        public:

            typedef geometry::model::Point<double_t> point_type;
            typedef std::unordered_map<int64_t, model::Boundary> boundaries_type;

        private:
            
            boundaries_type m_boundaries;
            int32_t m_territory_level;
            int32_t m_bonus_level;
            double_t m_epsilon;

            void filter(boundaries_type& boundaries, int32_t admin_level) const
            {
                for(auto it = boundaries.begin(); it != boundaries.end();)
                {
                    if (it->second.level != admin_level)
                    {
                        boundaries.erase(it++);
                    }
                    else
                    {
                        ++it;
                    }
                }
            }

            void compress(boundaries_type& boundaries, double_t epsilon) const
            {
                if (epsilon <= 0.0)
                    return;
                for (auto& [k, v] : boundaries)
                {
                    for (auto& polygon : v.geometry.polygons)
                    {
                        // Compress outer ring
                        geometry::algorithm::compress(polygon.outer, polygon.outer, epsilon);
                        // Compress inner rings
                        for (auto inner : polygon.inners)
                        {
                            geometry::algorithm::compress(inner, inner, epsilon);
                        }
                    }
                }
            }

        public:

            Builder() {};
            ~Builder() {};

            /* Setters */

            void set_boundaries(std::vector<model::Boundary>& boundaries)
            {
                this->m_boundaries = {};
                for (const model::Boundary& b : boundaries)
                {
                    this->m_boundaries[b.id] = b;
                }
            }

            void set_territory_level(int32_t level)
            {
                this->m_territory_level = level;
            }

            void set_bonus_level(int32_t level)
            {
                this->m_bonus_level = level;
            }

            void set_epsilon(double_t epsilon)
            {
                this->m_epsilon = epsilon;
            }

            /* Methods */

            model::Map build() const
            {
                // TODO Compression, Relationships, center points, polygon checks, troop circle position, etc.

                // Filter territories
                boundaries_type territories(m_boundaries);
                this->filter(territories, this->m_territory_level);
                this->compress(territories, this->m_epsilon);

                // Filter bonus links
                // boundaries_type bonus_links;
                // this->filter(bonus_links, this->m_bonus_level);

                return model::Map{ territories };
            }
            
        };

    }

}

#endif
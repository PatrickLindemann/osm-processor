#ifndef MAPMAKER_BUILDER_HPP
#define MAPMAKER_BUILDER_HPP

#include <cassert>
#include <initializer_list>
#include <vector>

#include "geometry/model.hpp"
#include "mapmaker/model.hpp"
#include "geometry/algorithm.hpp"
#include "geometry/transform.hpp"

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
            
            int32_t m_width;
            int32_t m_height;
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
                        for (auto& inner : polygon.inners)
                        {
                            geometry::algorithm::compress(inner, inner, epsilon);
                        }
                    }
                }
            }

            void project(boundaries_type& boundaries, geometry::transform::Projection<double_t> projection) const
            {
                for (auto& [k, v] : boundaries)
                {
                    for (auto& polygon : v.geometry.polygons)
                    {
                        // Project outer ring
                        for (auto& point : polygon.outer)
                        {
                            point = { projection.project(point.x, point.y) };
                            point.x *= m_width;
                            point.y *= m_height;
                        }
                        // Project inner rings
                        for (auto& inner : polygon.inners)
                        {
                            for (auto& point : inner)
                            {
                                point = { projection.project(point.x, point.y) };
                                point.x *= m_width;
                                point.y *= m_height;
                            }
                        }
                    }
                }
            }

        public:

            Builder() {};

            ~Builder() {};

            /* Setters */

            void set_territory_level(int32_t level)
            {
                m_territory_level = level;
            }

            void set_bonus_level(int32_t level)
            {
                m_bonus_level = level;
            }

            void set_boundaries(std::vector<model::Boundary>& boundaries)
            {
                m_boundaries = {};
                for (const model::Boundary& b : boundaries)
                {
                    m_boundaries[b.id] = b;
                }
            }

            void set_width(int32_t width)
            {
                m_width = width;
            }

            void set_height(int32_t height)
            {
                m_height = height;
            }

            void set_epsilon(double_t epsilon)
            {
                m_epsilon = epsilon;
            }

            /* Methods */

            model::Map build()
            {
                // Filter territory boundaries
                boundaries_type territories(m_boundaries);
                filter(territories, m_territory_level);

                // Filter bonus link boundaries
                boundaries_type bonus_links(m_boundaries);
                filter(bonus_links, m_bonus_level);
                
                // Compress territories and bonus links
                compress(territories, m_epsilon);
                compress(bonus_links, m_epsilon);

                // Calculate map boundaries
                geometry::model::Rectangle<double_t> map_bounds = { DBL_MAX, DBL_MAX, -DBL_MAX, -DBL_MAX };
                for (const auto& [k, v] : territories)
                {
                    map_bounds.extend(geometry::algorithm::bounds(v.geometry));
                }
                for (const auto& [k, v] : bonus_links)
                {
                    map_bounds.extend(geometry::algorithm::bounds(v.geometry));
                }

                // Set dimensions
                if (m_width == 0)
                {
                    m_width = map_bounds.width() / map_bounds.height() * m_height;
                }
                else
                {
                    m_height = map_bounds.height() / map_bounds.width() * m_width;
                }

                // Create the map bounds interval for the projection
                geometry::transform::Interval map_interval{
                    map_bounds.min.x,
                    map_bounds.min.y,
                    map_bounds.max.x,
                    map_bounds.max.y
                };            
                
                // Project the territory and bonus link points
                geometry::transform::WebMercatorProjection projection{ map_interval };
                project(territories, projection);
                project(bonus_links, projection);

                // TODO: Relationships, center points, polygon checks, troop circle position, etc.

                // Create and return the map
                return model::Map{ m_width, m_height, territories, bonus_links };
            }
            
        };

    }

}

#endif
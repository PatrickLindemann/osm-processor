#ifndef MAPMAKER_BUILDER_HPP
#define MAPMAKER_BUILDER_HPP

#include <vector>
#include <cassert>
#include <initializer_list>
#include <iostream>

#include "geometry/model.hpp"
#include "geometry/algorithm.hpp"
#include "geometry/transform.hpp"
#include "geometry/util.hpp"
#include "mapmaker/model.hpp"

namespace mapmaker
{

    namespace builder
    {

        class Builder
        {
        public:

            typedef geometry::model::Point<double_t> point_t;
            typedef std::unordered_map<int64_t, model::Boundary> boundaries_t;

        private:
            
            int32_t m_width;
            int32_t m_height;
            boundaries_t m_boundaries;
            int32_t m_territory_level;
            int32_t m_bonus_level;
            double_t m_epsilon;

            void filter(boundaries_t& boundaries, int32_t admin_level) const
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

            void compress(boundaries_t& boundaries, double_t epsilon) const
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

            void convert_to_radians(boundaries_t& boundaries) const
            {
                for (auto& [k, v] : boundaries)
                {
                    for (auto& polygon : v.geometry.polygons)
                    {
                        // Convert outer ring
                        for (auto& point : polygon.outer)
                        {
                            point = point_t{
                                geometry::util::radians(point.x),
                                geometry::util::radians(point.y)
                            };
                        }
                        // Convert inner rings
                        for (auto& inner : polygon.inners)
                        {
                            for (auto& point : inner)
                            {
                                point = point_t{
                                    geometry::util::radians(point.x),
                                    geometry::util::radians(point.y)
                                };
                            }
                        }
                    }
                }
            }

            void project(boundaries_t& boundaries, geometry::transform::Projection<double_t>& projection) const
            {
                for (auto& [k, v] : boundaries)
                {
                    for (auto& polygon : v.geometry.polygons)
                    {
                        // Project outer ring
                        for (auto& point : polygon.outer)
                        {
                            point = projection.project(point);
                        }
                        // Project inner rings
                        for (auto& inner : polygon.inners)
                        {
                            for (auto& point : inner)
                            {
                                point = projection.project(point);
                            }
                        }
                    }
                }
            }

            void scale(boundaries_t& boundaries, int32_t width, int32_t height) const
            {
                for (auto& [k, v] : boundaries)
                {
                    for (auto& polygon : v.geometry.polygons)
                    {
                        // Scale outer ring
                        for (auto& point : polygon.outer)
                        {
                            point.x *= width;
                            point.y *= height;
                        }
                        // Scale inner rings
                        for (auto& inner : polygon.inners)
                        {
                            for (auto& point : inner)
                            {
                                point.x *= width;
                                point.y *= height;
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
                boundaries_t territories(m_boundaries);
                filter(territories, m_territory_level);

                // Filter bonus link boundaries
                boundaries_t bonus_links(m_boundaries);
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
                int32_t width, height;
                if (m_width == 0)
                {
                    width = map_bounds.width() / map_bounds.height() * m_height;
                    height = m_height;
                }
                else
                {
                    width = m_width;
                    height = map_bounds.height() / map_bounds.width() * m_width;
                }
                            
                // Prepare the map projection
                // geometry::transform::IdentityProjection map_projection{};
                geometry::transform::MercatorProjection map_projection{};
                // geometry::transform::CylindricalEqualAreaProjection map_projection{};

                // Prepare the scaling projection
                point_t min = map_projection.project(point_t{
                    geometry::util::radians(map_bounds.min.x),
                    geometry::util::radians(map_bounds.min.y)
                });
                point_t max = map_projection.project(point_t{
                    geometry::util::radians(map_bounds.max.x),
                    geometry::util::radians(map_bounds.max.y)
                });
                geometry::transform::Interval map_interval{ min, max };
                geometry::transform::UnitProjection unit_projection{ map_interval };

                // Apply the projections 
                convert_to_radians(territories);
                project(territories, map_projection);
                project(territories, unit_projection);
                scale(territories, width, height);
                
                // TODO: Relationships, center points, polygon checks, troop circle position, etc.

                // Create and return the map
                return model::Map{ width, height, territories, bonus_links };
            }
            
        };

    }

}

#endif
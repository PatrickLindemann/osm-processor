#pragma once

#include <algorithm>
#include <vector>
#include <unordered_map>

#include "model/geometry/multipolygon.hpp"
#include "model/geometry/polygon.hpp"
#include "model/geometry/rectangle.hpp"
#include "model/map/map.hpp"
#include "model/map/bonus.hpp"
#include "model/map/territory.hpp"
#include "model/memory/area.hpp"
#include "model/memory/buffer.hpp"
#include "model/type.hpp"

namespace mapmaker
{

    namespace builder
    {

        using namespace model;

        /**
         * A builder that creates a map with territories and
         * boundaries from an area buffer.
         */
        class MapBuilder
        {

            /* Members */

            const memory::Buffer<memory::Area>& m_areas;
            const memory::Buffer<memory::Node>& m_nodes;

        public:

            /* Constructors */

            MapBuilder(
                const memory::Buffer<memory::Area>& areas,
                const memory::Buffer<memory::Node>& nodes
            ) : m_areas(areas), m_nodes(nodes) {}

        protected:

            /* Helper methods */

            /**
             * Convert a memory ring of node references to a ring geometry
             * with points.
             * 
             * @param ring The memory ring
             * @returns    The ring geometry
             * 
             * Time complexity: Linear
             */
            geometry::Ring<double> create_ring(
                const memory::Ring& ring
            ) {
                geometry::Ring<double> geometry;
                for (const memory::NodeRef& node : ring)
                {
                    geometry.push_back(m_nodes.at(node).point());
                }
                return geometry;
            }

            /**
             * Convert an outer and N inner memory rings of node references
             * to a polygon geometry with points.
             * 
             * @param outer  The outer memory ring
             * @param inners The inner memory rings
             * @returns      The polygon geometry
             * 
             * Time complexity: Linear
             */
            geometry::Polygon<double> create_polygon(
                const memory::Ring& outer,
                const std::vector<memory::Ring>& inners
            ) {
                geometry::Polygon<double> geometry{ create_ring(outer) };
                for (const memory::Ring& inner : inners)
                {
                    geometry.inners().push_back(create_ring(inner));
                }
                return geometry;
            }

            /**
             * Convert a complex area with node references to a multipolygon
             * geometry with points.
             * 
             * @param area The memory area
             * @returns    The multipolygon geometry
             * 
             * Time complexity: Linear
             */
            geometry::MultiPolygon<double> create_multipolygon(
                const memory::Area& area
            ) {
                geometry::MultiPolygon<double> geometry{};
                for (const memory::Ring& outer : area.outer_rings())
                {
                    // Create a polygon for each outer ring
                    geometry::Polygon<double> polygon { create_ring(outer) };
                    for (const memory::Ring& inner : area.inner_rings(outer))
                    {
                        polygon.inners().push_back(create_ring(inner));
                    }
                    // Add polygon to multipolygon
                    geometry.polygons().push_back(polygon);
                }
                return geometry;
            }

        public:
            
            /* Methods */

            /**
             * 
             */
            map::Map build_map(
                int width,
                int height,
                level_type territory_level,
                const std::vector<level_type>& bonus_levels
            ) {
                // Create the map with the specified width and height
                map::Map map;
                map.width() = width;
                map.height() = height;

                // Determine the lowest bonus level
                level_type min_bonus_level;
                if (!bonus_levels.empty())
                {
                    min_bonus_level = *std::min_element(bonus_levels.begin(), bonus_levels.end());
                }

                // Convert the areas from the area buffer
                for (const memory::Area& area : m_areas)
                {
                    if (area.level() == territory_level)
                    {
                        // Area is a territory area
                        assert(area.outer_rings().size() == 1);

                        // Retrieve outer and inner rings from the area
                        const memory::Ring& outer = area.outer_rings().at(0);
                        const std::vector<memory::Ring>& inners = area.inner_rings(outer);

                        // Create the territory and convert the geometry
                        map::Territory territory( map.territories().size() );
                        territory.name() = area.name();
                        territory.geometry() = create_polygon(outer, inners);

                        // Add the territory to the map
                        map.territories().push_back(territory);
                    }
                    else if (area.level() == min_bonus_level)
                    {
                        // Area is a regular bonus area
                        assert(area.outer_rings().size() >= 1);

                        // Create the bonus and convert the geometry
                        map::RegularBonus bonus( map.bonuses().size() );
                        bonus.name() = area.name();
                        bonus.geometry() = create_multipolygon(area);

                        // Add the bonus to the map
                        map.bonuses().push_back(bonus);
                    }
                    else
                    {
                        // Area is a super bonus area
                        assert(area.outer_rings().size() >= 1);

                        // Create the bonus and convert the geometry
                        map::SuperBonus bonus( map.bonuses().size() );
                        bonus.name() = area.name();
                        bonus.geometry() = create_multipolygon(area);

                        // Add the bonus to the map
                        map.bonuses().push_back(bonus);
                    }
                }

                return map;
            } 

        };

        /*
        class MapBuilder
        {
                   protected:

            std::string get_random_hex_color()
            {
                return util::hsl_to_hex(rand() % 361, 1, 1);
            }

        public:

            void run() {
                for (const memory::Area<T>& area : m_areas)
                {
                    multipolygon_type multipolygon;
                    for (const memory::Ring<T>& outer : area.outer_rings())
                    {
                        polygon_type polygon;
                        polygon.outer = convert_ring(outer);
                        for (const memory::Ring<T> inner : area.inner_rings(outer))
                        {
                            polygon.inners.push_back(convert_ring(outer));
                        }
                        multipolygon.polygons.push_back(polygon);
                    }
                    m_geometries.push_back(multipolygon);
                }
            }

            void run()
            {
                // Seed the time
                srand(time(NULL));

                // Prepare the id maps for territories and bonuses
                std::unordered_map<id_type, id_type> ids;
                size_t t_id = 0;
                size_t b_id = 0;
                for (const memory::Area<T>& area : m_data.areas)
                {
                    if (area.level() == m_data.territory_level)
                    {
                        ids[area.id()] = t_id;
                        ++t_id;
                    }
                    else
                    {
                        ids[area.id()] = b_id;
                        ++b_id;
                    }
                }

                // Re-Index and cache neighbor and neighbor lists
                std::unordered_map<id_type, std::vector<id_type>> neighbors;
                std::unordered_map<id_type, std::vector<id_type>> children;
                for (const memory::Area<T>& area : m_data.areas)
                {
                    if (area.level() == m_data.territory_level)
                    {
                        // Map neighbor ids
                        std::vector<id_type> adjacents = m_data.neighbors.adjacents(area.id());
                        for (size_t i = 0; i < adjacents.size(); i++)
                        {
                            adjacents.at(i) = ids.at(adjacents.at(i));
                        }
                        neighbors[area.id()] = adjacents;
                    }
                    else
                    {
                        // Map subarea ids
                        std::vector<id_type> subareas = m_data.subareas.at(area.id());
                        for (size_t i = 0; i < subareas.size(); i++)
                        {
                            subareas.at(i) = ids.at(subareas.at(i));
                        }
                        children[area.id()] = subareas;
                    }
                }

                // Determine minimum bonus level
                unsigned short min_bonus_level = *std::min_element(m_data.bonus_levels);

                // Assemble territories and bonuses
                for (const memory::Area<T>& area : m_data.areas)
                {
                    if (area.level() == m_data.territory_level)
                    {
                        // Create territory
                        map::Territory<T> territory{
                            ids.at(area.id()),
                            area.name(),
                            m_data.centerpoints.at(area.id()),
                            neighbors.at(area.id(),
                            m_data.geometries.at(area.id())
                        };
                        m_territories.push_back(territory);
                    }
                    else
                    {
                        // Calculate number of 
                        // Create regular bonus
                        map::Bonus<T> bonus{
                            m_bonuses.size(),
                            area.name(),
                            0,
                            get_random_hex_color(),
                            area.level() != min_bonus_level,
                            children.at(area.id())
                        };
                    }
                }
            }
            */

    }

}
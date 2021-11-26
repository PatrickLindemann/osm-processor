#pragma once

#include "util/color.hpp"
#include "util/rand.hpp"

using namespace model;

namespace mapmaker
{

    template <typename T>
    class MapBuilder
    {

        /* Members */

        std::string m_name = "";

        std::size_t m_width  = 0;
        std::size_t m_height = 0;

        level_type m_territory_level   = 0;
        level_type m_bonus_level       = 0;
        level_type m_super_bonus_level = 0;

        graph::UndirectedGraph m_neighbors = {};

        std::map<object_id_type, std::set<object_id_type>> m_hierarchy = {};

        std::map<object_id_type, object_id_type> m_ids = {};

    public:

        /* Constructors */

        MapBuilder() {}

        /* Setters */

        void name(std::string name)
        {
            m_name = name;
        }

        void width(std::size_t width)
        {
            m_width = width;
        }

        void height(std::size_t height)
        {
            m_height = height;
        }

        void territory_level(level_type level)
        {
            m_territory_level = level;
        }

        void bonus_level(level_type level)
        {
            m_bonus_level = level;
        }

        void super_bonus_level(level_type level)
        {
            m_super_bonus_level = level;
        }

        void neighbors(const graph::UndirectedGraph& neighbors)
        {
            m_neighbors = neighbors;
        }

        void hierarchy(const std::map<object_id_type, std::set<object_id_type>>& hierarchy)
        {
            m_hierarchy = hierarchy;
        }

    protected:

        /* Helper methods */

        void translate(geometry::Point<T>& point)
        {
            point.y() = m_height - point.y();
        }

        void translate(geometry::Ring<T>& ring)
        {
            for (geometry::Point<T>& point : ring)
            {
                translate(point);
            }
        }

        void translate(geometry::MultiPolygon<T>& geometry)
        {
            for (geometry::Polygon<T>& polygon : geometry.polygons())
            {
                translate(polygon.outer());
                for (geometry::Ring<T>& inner : polygon.inners())
                {
                    translate(inner);
                }
            }
        }

        std::string random_color()
        {
            int h = util::rand_between(0, 36) * 10;
            float s = 1.0f;
            float l = util::rand_between(0.5f, 1.0f);
            return util::hsl_to_hex(h, s, l);
        }

        warzone::Territory<T> territory(const Boundary<T>& boundary)
        {
            // Create the territory
            warzone::Territory<T> territory{
                m_ids.at(boundary.id),
                boundary.name,
                boundary.geometry,
                boundary.center
            };
            // Add the neighbors
            for (const object_id_type& neighbor : m_neighbors.adjacents(boundary.id))
            {
                territory.neighbors.push_back(m_ids.at(neighbor));
            }
            return territory;
        }

        warzone::Bonus<T> bonus(const Boundary<T>& boundary)
        {
            // Create the bonus
            warzone::Bonus<T> bonus{
                m_ids.at(boundary.id),
                boundary.name,
                boundary.geometry,
                boundary.center,
                1, // TODO
                random_color()
            };
            // Add the children
            if (m_hierarchy.count(boundary.id))
            {
                for (const object_id_type& child : m_hierarchy.at(boundary.id))
                {
                    bonus.children.push_back(m_ids.at(child));
                }
            }
            return bonus;
        }

        warzone::SuperBonus<T> super_bonus(const Boundary<T>& boundary)
        {
            // Create the super bonus
            warzone::SuperBonus<T> super_bonus{
                m_ids.at(boundary.id),
                boundary.name,
                boundary.geometry,
                boundary.center,
                1, // TODO
                random_color()
            };
            // Add the children
            if (m_hierarchy.count(boundary.id))
            {
                for (const object_id_type& child : m_hierarchy.at(boundary.id))
                {
                    super_bonus.children.push_back(m_ids.at(child));
                }
            }
            return super_bonus;
        }

    public:

        /* Methods */

        warzone::Map<T> run(std::map<object_id_type, Boundary<T>>& boundaries)
        {
            // Create the total set of levels
            std::set<level_type> levels{ m_territory_level };
            if (m_bonus_level > 0)
            {
                levels.insert(m_bonus_level);
            }
            if (m_super_bonus_level > 0)
            {
                levels.insert(m_super_bonus_level);
            }

            // Create the map instance
            warzone::Map<T> map{
                m_name,
                m_width,
                m_height,
                levels
            };

            // Translate the boundaries into the svg coordinate system
            for (auto& [id, boundary] : boundaries)
            {
                translate(boundary.geometry);
                translate(boundary.center);
            }

            // Fill the id map, which maps the boundary ids to territory, bonus
            // and super bonus ids.
            object_id_type t = 1;
            object_id_type b = 1;
            object_id_type s = 1;
            for (const auto& [id, boundary] : boundaries)
            {
                if (boundary.level == m_territory_level)
                {
                    m_ids[id] = t++;
                }
                else if (boundary.level == m_bonus_level)
                {
                    m_ids[id] = b++;
                }
                else if (boundary.level == m_super_bonus_level)
                {
                    m_ids[id] = s++;
                }
            }

            // Create the territories, bonuses and super bonuses depending on
            // the boundary level
            for (const auto& [id, boundary] : boundaries)
            {
                if (boundary.level == m_territory_level)
                {
                    map.territories.push_back(territory(boundary));
                }
                else if (boundary.level == m_bonus_level)
                {
                    map.bonuses.push_back(bonus(boundary));
                }
                else if (boundary.level == m_super_bonus_level)
                {
                    map.super_bonuses.push_back(super_bonus(boundary));
                }
            }

            return map;
        }

    };

}
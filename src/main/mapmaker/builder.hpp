#pragma once

#include <algorithm>
#include <vector>
#include <unordered_map>

#include "model/container.hpp"
#include "model/geometry/line.hpp"
#include "model/geometry/multipolygon.hpp"
#include "model/geometry/rectangle.hpp"
#include "model/graph/undirected_graph.hpp"
#include "model/map/map.hpp"
#include "model/map/territory.hpp"
#include "model/map/bonus.hpp"
#include "model/memory/area.hpp"
#include "model/memory/buffer.hpp"
#include "model/memory/types.hpp"
#include "util/misc.hpp"

namespace mapmaker
{

    namespace builder
    {

        using namespace model;

        template <typename T>
        class MapBuilder
        {
        public:

            using id_type = memory::object_id_type;

        protected:

            const DataContainer<T>& m_data;
            
            std::vector<map::Territory<T>> m_territories;
            std::vector<map::Territory<T>> m_bonuses;

        public:

            MapBuilder(const DataContainer<T>& data) : m_data(data) {};

            map::Map<T> map()
            {
                return map::Map<T>{
                    m_data.name,
                    m_data.width,
                    m_data.height,
                    m_territories,
                    m_bonuses
                };
            }

        protected:

            std::string get_random_hex_color()
            {
                return util::hsl_to_hex(rand() % 361, 1, 1);
            }

        public:

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
            
        };

    }

}
#pragma once

#include <nlohmann/json.hpp>

#include "io/reader/reader.hpp"
#include "model/warzone/map.hpp"

namespace io
{

    using json = nlohmann::json;
    using namespace model;

    /**
     * A reader that retreives general file information for an OSM file.
     */
    template <typename T>
    class MapdataReader : public Reader<model::warzone::Map<T>>
    {
    protected:

        /* Members */

        /**
         * 
         */
        warzone::Map<T> m_map;

    public:

        /* Constructors */

        MapdataReader(fs::path file_path) : Reader<warzone::Map<T>>(file_path) {}

    protected:

        /* Helper methods */

        /**
         *
         */
        void add_territory(const json::object_t& obj)
        {
            warzone::Territory<T> territory{};
            territory.id = obj.at("id");
            territory.name = obj.at("name");
            territory.center = {
                obj.at("center").at("x"),
                obj.at("center").at("y")
            };
            for (const object_id_type& neighbor : obj.at("neighbors"))
            {
                territory.neighbors.push_back({ neighbor });
            }
            m_map.territories.push_back(territory);
        }

        /**
         *
         */
        void add_bonus(const json::object_t& obj)
        {
            warzone::Bonus<T> bonus{};
            bonus.id = obj.at("id");
            bonus.name = obj.at("name");
            bonus.color = obj.at("color");
            bonus.armies = obj.at("armies");
            for (const object_id_type& child : obj.at("children"))
            {
                bonus.children.push_back({ child });
            }
            m_map.bonuses.push_back(bonus);
        }

        /**
         *
         */
        void add_super_bonus(const json::object_t& obj)
        {
            warzone::SuperBonus<T> super_bonus{};
            super_bonus.id = obj.at("id");
            super_bonus.name = obj.at("name");
            super_bonus.color = obj.at("color");
            super_bonus.armies = obj.at("armies");
            for (const object_id_type& child : obj.at("children"))
            {
                super_bonus.children.push_back({ child });
            }
            m_map.super_bonuses.push_back(super_bonus);
        }

    public:

        /* Override Methods */

        warzone::Map<T> read() override
        {
            // Read the json from the specified file path
            std::ifstream ifs { this->m_path };
            json data = json::parse(ifs);    
            
            // Parse the primitive json values
            m_map.name = data.at("name");

            // Parse the territory information
            for (const auto& territory : data.at("territories"))
            {
                add_territory(territory);
            }

            // Parse the super bonus information
            for (const auto& bonus : data.at("bonuses"))
            {
                add_bonus(bonus);
            }

            // Parse the super bonus information
            for (const auto& super_bonus : data.at("super_bonuses"))
            {
                add_super_bonus(super_bonus);
            }
            
            // Return the resulting map container
            return m_map;
        }

    };

}
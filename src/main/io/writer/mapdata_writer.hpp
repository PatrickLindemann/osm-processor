#pragma once

#include <nlohmann/json.hpp>

#include "io/writer/writer.hpp"

#include "model/warzone/map.hpp"

#include "util/time.hpp"

namespace io
{

    using json = nlohmann::json;
    using namespace model;

    /**
     * A reader for API configuration JSON files.
     */
    template <typename T>
    class MapdataWriter : public Writer<warzone::Map<T>>
    {
    protected:

        /* Members */

        json m_data;

    public:

        /* Constructors */

        MapdataWriter(fs::path file_path) : Writer<warzone::Map<T>>(file_path) {}
        
    protected:

        /* Helper Methods */

        void write_territory(const warzone::Territory<T>& territory)
        {
            auto obj = json::object();
            obj["id"] = territory.id;
            obj["name"] = territory.name;
            obj["center"] = json::object();
            obj["center"]["x"] = territory.center.x();
            obj["center"]["y"] = territory.center.y();
            obj["neighbors"] = json::array();
            for (const object_id_type& neighbor : territory.neighbors)
            {
                obj["neighbors"].push_back(neighbor);
            }
            m_data["territories"].push_back(obj);
        }

        void write_bonus(const warzone::Bonus<T>& bonus)
        {
            auto obj = json::object();
            obj["id"] = bonus.id;
            obj["name"] = bonus.name;
            obj["color"] = bonus.color;
            obj["armies"] = bonus.armies;
            obj["children"] = json::array();
            for (const object_id_type& child : bonus.children)
            {
                obj["children"].push_back(child);
            }
            m_data["bonuses"].push_back(obj);
        }

        void write_super_bonus(const warzone::SuperBonus<T>& super_bonus)
        {
            auto obj = json::object();
            obj["id"] = super_bonus.id;
            obj["name"] = super_bonus.name;
            obj["color"] = super_bonus.color;
            obj["armies"] = super_bonus.armies;
            obj["children"] = json::array();
            for (const object_id_type& child : super_bonus.children)
            {
                obj["children"].push_back(child);
            }
            m_data["super_bonuses"].push_back(obj);
        }

    public:

        /* Override Methods */

        void write(model::warzone::Map<T>&& map) override
        {
            std::ofstream ofs{ this->m_path, std::ios::trunc };
            ofs.precision(4);

            // Prepare the json data and add the json headers
            m_data["name"] = map.name;
            m_data["created_at"] = util::get_current_iso_timestamp();
            m_data["levels"] = map.levels;

            // Add the territories
            m_data["territories"] = json::array();
            for (const warzone::Territory<T>& territory : map.territories)
            {
                write_territory(territory);
            }

            // Add the bonuses
            m_data["bonuses"] = json::array();
            for (const warzone::Bonus<T>& bonus : map.bonuses)
            {
                write_bonus(bonus);
            }

            // Add the super bonuses
            m_data["super_bonuses"] = json::array();
            for (const warzone::SuperBonus<T>& super_bonus : map.super_bonuses)
            {
                write_super_bonus(super_bonus);
            }

            // Write json document to file
            ofs << m_data.dump() << std::endl;
        }

    };

}
#pragma once

#include <linux/prctl.h>
#include <nlohmann/json.hpp>

#include "http/request.hpp"
#include "model/config.hpp"
#include "model/warzone/map.hpp"

namespace http
{

    using json = nlohmann::ordered_json;
    using namespace model;

    /**
     * A simple request wrapper for metadata upload requests.
     */
    template <typename T>
    class MapdataRequest : public Request
    {       
    protected:

        /* Members */

        json m_data;

        /* Methods */

        /**
         * 
         */
        void add_name(const warzone::Territory<T>& territory)
        {
            auto command = json::object();
            command["command"] = "setTerritoryName";
            command["id"] = territory.id;
            command["name"] = territory.name;
            m_data["commands"].push_back(command);
        }

        /**
         * 
         */
        void add_center(const warzone::Territory<T>& territory)
        {
            auto command = json::object();
            command["command"] = "setTerritoryCenterPoint";
            command["id"] = territory.id;
            command["x"] = territory.center.x();
            command["y"] = territory.center.y();
            m_data["commands"].push_back(command);
        }

        /**
         * 
         */
        void add_connection(const warzone::Territory<T>& territory, model::object_id_type neighbor)
        {
            auto command = json::object();
            command["command"] = "addTerritoryConnection";
            command["id1"] = territory.id;
            command["id2"] = neighbor;
            command["wrap"] = "Normal";
            m_data["commands"].push_back(command);
        }

        /**
         * 
         */
        void add_bonus(const warzone::Bonus<T>& bonus)
        {
            auto command = json::object();
            command["command"] = "addBonus";
            command["name"] = bonus.name;
            command["armies"] = bonus.armies;
            command["color"] = bonus.color;
            m_data["commands"].push_back(command);
        }

        /**
         * 
         */
        void add_territory_to_bonus(const warzone::Bonus<T>& bonus, model::object_id_type child)
        {
            auto command = json::object();
            command["command"] = "addTerritoryToBonus";
            command["bonusName"] = bonus.name;
            command["id"] = child;
            m_data["commands"].push_back(command);
        }

    public:

        /* Constructors */

        /*
         *
         */
        MapdataRequest(const warzone::Map<T>& map, const Config& config, long map_id)
        {
            // Add the authentication information and map id
            m_data["mapID"] = map_id;
            m_data["email"] = config.email;
            m_data["APIToken"] = config.api_token;

            // Prepare the command array
            m_data["commands"] = json::array();

            // Add the territory commands
            for (const warzone::Territory<T>& territory : map.territories)
            {
                add_name(territory);
                add_center(territory);
                for (const model::object_id_type& neighbor : territory.neighbors)
                {
                    add_connection(territory, neighbor);
                }
            }

            // Add the bonus commands
            for (const warzone::Bonus<T>& bonus : map.bonuses)
            {
                add_bonus(bonus);
                for (const model::object_id_type& child : bonus.children)
                {
                    add_territory_to_bonus(bonus, child);
                }
            }
            
            // Add the super bonus commands
            for (const warzone::SuperBonus<T>& super_bonus : map.super_bonuses)
            {
                // Ignore for now, as Warzone currently doesn't support the
                // creation of super bonuses through the API.
            }

        }

        /* Accessors */
        
        const std::string payload() const
        {
            return m_data.dump();
        }

    };

}
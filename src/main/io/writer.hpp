#pragma once

#include <cfloat>
#include <cmath>
#include <string>
#include <iostream>
#include <fstream>
#include <cassert>
#include <type_traits>

#include <nlohmann/json.hpp>

#include "model/container.hpp"
#include "model/geometry/multipolygon.hpp"
#include "model/map/bonus.hpp"
#include "model/map/map.hpp"
#include "model/map/territory.hpp"
#include "util/time.hpp"

using json = nlohmann::ordered_json;

using namespace model;

namespace io
{

    namespace writer
    {
        
        namespace detail
        {

            template <typename StreamType, typename T>
            void write_geometry(StreamType& stream, const geometry::Polygon<T>& geometry)
            {
                // Add outer points (clockwise)
                stream << "M ";
                for (auto it = geometry.outer().begin(); it != geometry.outer().end(); ++it)
                {   
                    if (it == geometry.outer().begin() + 1)
                    {
                        stream << "L ";
                    }
                    stream << it->x() << " " << it->y() << " ";
                }
                stream << "Z";
                if (geometry.inners().size() > 0)
                {
                    // Add inner points (counter-clockwise)
                    for (const geometry::Ring<T>& inner : geometry.inners())
                    {
                        stream << " M ";
                        for (auto it = inner.rbegin(); it != inner.rend(); ++it)
                        {   
                            if (it == inner.rbegin() + 1)
                            {
                                stream << "L ";
                            }
                            stream << it->x() << " " << it->y() << " ";
                        }
                        stream << "Z";
                    }
                }
            }

            template <typename StreamType, typename T>
            void write_geometry(StreamType& stream, const geometry::MultiPolygon<T>& geometry)
            {
                for (auto it = geometry.polygons().begin(); it != geometry.polygons().end(); it++)
                {
                    if (it != geometry.polygons().begin())
                    {
                        stream << " ";
                    }
                    write_geometry(stream, *it);
                }
            }

        }

        /**
         * 
         */
        void write_config(std::string file_path, const ConfigContainer& config)
        {
            std::ofstream ofs{ file_path, std::ios::trunc };

            // Prepare and fille the json data
            json data;
            data["email"] = config.email;
            data["api-token"] = config.api_token;
            
            // Write the json to the output stream
            ofs << data.dump() << std::endl;
        }

        /**
         * 
         */
        void write_metadata(std::string file_path, const map::Map& map)
        {
            std::ofstream ofs{ file_path, std::ios::trunc };
            ofs.precision(4);

            // Prepare the json data and add the json headers
            json data;
            data["name"] = map.name();
            data["created_at"] = util::get_current_iso_timestamp();
            data["levels"] = map.levels();

            // Add the territories
            auto territories = json::array();
            for (const map::Territory& t : map.territories())
            {
                // Create territory json
                auto territory = json::object();
                territory["id"] = t.id();
                territory["name"] = t.name();
                auto center = json::object();
                center["x"] = t.center().x();
                center["y"] = t.center().y();
                territory["center"] = center;
                auto neighbors = json::array();
                for (const map::TerritoryRef& neighbor : t.neighbors())
                {
                    neighbors.push_back(neighbor.ref());
                }
                territory["neighbors"] = neighbors;
                // Add the json territory to the territories array
                territories.push_back(territory);
            }
            data["territories"] = territories;

            // Add the bonuses
            auto bonuses = json::array();
            for (const map::Bonus& b : map.bonuses())
            {
                // Create territory json
                auto bonus = json::object();
                bonus["id"] = b.id();
                bonus["name"] = b.name();
                bonus["color"] = b.color();
                bonus["armies"] = b.armies();
                bonus["is_super"] = b.is_super();
                auto children = json::array();
                for (const map::BoundaryRef& child : b.children())
                {
                    children.push_back(child.ref());
                }
                bonus["children"] = children;
                // Add the json territory to the territories array
                bonuses.push_back(bonus);
            }
            data["bonuses"] = bonuses;

            // Write json document to file
            ofs << data.dump() << std::endl;
        }

        /**
         * 
         */
        void write_map(std::string file_path, const map::Map& map)
        {
            std::ofstream ofs{ file_path, std::ios::trunc };
            ofs.precision(4);

            if (!ofs.is_open())
            {
                return; // TODO Error handling
            }

            // Write headers
            ofs << "<svg xmlns=\"http://www.w3.org/2000/svg\" "
            << "id=\"my-svg\" "
            << "width=\"" << map.width() << "px\" "
            << "height=\"" << map.height() << "px\""
            << ">";

            // Write Connections
            // TODO

            // Write bonuses in their order first
            // TODO with hirarchy
            for (const map::Bonus& bonus : map.bonuses())
            {
                ofs << "<path "
                    << "style=\"fill: " << bonus.color() << "; stroke:black; stroke-width: 2px;\" "
                    << "d=\"";
                detail::write_geometry(ofs, bonus.geometry());
                ofs << "\"/>"; // End path
            }

            // Write territories
            for (const map::Territory& territory : map.territories())
            {
                ofs << "<path "
                    << "id=\"Territory_" << territory.id() << "\" "
                    << "style=\"stroke:black; fill:none; stroke-width: 1px;\" "
                    << "d=\"";
                detail::write_geometry(ofs, territory.geometry());
                ofs << "\"/>"; // End path
            }

            // Write centers
            for (const map::Territory& territory : map.territories())
            {
                ofs << "<circle "
                    << "id=\"Center_" << territory.id() << "\" "
                    << "cx=\"" << territory.center().x() << "\" "
                    << "cy=\"" << territory.center().y() << "\" "
                    << "r=\"2\" "
                    << "fill=\"black\""
                    << "/>";
            }

            // Write Bonus Links
            // TODO

            ofs << "</svg>" << std::endl; // End file
        }
        
    }

}
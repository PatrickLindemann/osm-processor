#pragma once

#include <cfloat>
#include <cmath>
#include <string>
#include <iostream>
#include <fstream>
#include <cassert>
#include <type_traits>

#include "model/geometry/multipolygon.hpp"
#include "model/map/bonus.hpp"
#include "model/map/map.hpp"
#include "model/map/territory.hpp"
#include "model/config.hpp"
#include "util/time.hpp"
#include "util/join.hpp"

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
        void write_config(std::string file_path, const Config& config)
        {
            std::ofstream out{ file_path, std::ios::trunc };
            out << '{'
                << "\"email\":" << '"' << config.email << '"' << ','
                << "\"api-token\":" << '"' << config.api_token << '"'
                << '}'
                << std::endl;
        }

        /**
         * 
         */
        void write_metadata(std::string file_path, const map::Map& map)
        {
            std::ofstream out{ file_path, std::ios::trunc };
            out.precision(4);

            // Write headers
            out << "{"
                << "\"name\":" << '\"' << map.name() << '\"' << ','
                << "\"created\":" << '\"' << util::get_current_iso_timestamp() << '\"' << ','
                << "\"levels\":" << '[' << util::join(map.levels()) << ']' << ',';

            // Write boundary information
            out << "\"boundaries\":" << '{';
            // Write territories
            out << "\"territories\":" << '[';
            for (auto it = map.territories().cbegin(); it != map.territories().cend(); it++)
            {
                if (it != map.territories().cbegin())
                {
                    out << ',';
                }
                out << '{'
                    << "\"id\":" << it->id() << ','
                    << "\"name\":" << '\"' << it->name() << '\"' << ','
                    << "\"center\":" << '['
                        << it->center().x() << ','
                        << it->center().y()
                    << ']' << ','
                    << "\"neighbors\":" << '[' << util::join(it->neighbors()) << ']'
                    << '}';
            }
            out << ']'; // End territories
            // Write bonuses
            out << ',';
            out << "\"bonuses\":[";
            for (auto it = map.bonuses().cbegin(); it != map.bonuses().cend(); it++)
            {
                if (it != map.bonuses().cbegin())
                {
                    out << ',';
                }
                out << '{'
                    << "\"id\":" << it->id() << ','
                    << "\"name\":" << '\"' << it->name() << '\"' << ','
                    << "\"color\":" << '\"' << it->color() << '\"' << ','
                    << "\"armies\":" << '\"' << it->armies() << '\"' << ','
                    << "\"super_bonus\":" << '\"' << it->is_super() << '\"' << ','
                    << "\"children\":" << '[' << util::join(it->children()) << ']'
                    << '}';
            }
            out << ']'; // End bonues
            out << '}'; // End boundaries

            out << "}" << std::endl; // End file
        }

        /**
         * 
         */
        void write_map(std::string file_path, const map::Map& map)
        {
            std::ofstream out{ file_path, std::ios::trunc };
            out.precision(4);

            if (!out.is_open())
            {
                return; // TODO Error handling
            }

            // Write headers
            out << "<svg xmlns=\"http://www.w3.org/2000/svg\" "
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
                out << "<path "
                    << "style=\"fill: " << bonus.color() << "; stroke:black; stroke-width: 2px;\" "
                    << "d=\"";
                detail::write_geometry(out, bonus.geometry());
                out << "\"/>"; // End path
            }

            // Write territories
            for (const map::Territory& territory : map.territories())
            {
                out << "<path "
                    << "id=\"Territory_" << territory.id() << "\" "
                    << "style=\"stroke:black; fill:none; stroke-width: 1px;\" "
                    << "d=\"";
                detail::write_geometry(out, territory.geometry());
                out << "\"/>"; // End path
            }

            // Write centers
            for (const map::Territory& territory : map.territories())
            {
                out << "<circle "
                    << "id=\"Center_" << territory.id() << "\" "
                    << "cx=\"" << territory.center().x() << "\" "
                    << "cy=\"" << territory.center().y() << "\" "
                    << "r=\"2\" "
                    << "fill=\"black\""
                    << "/>";
            }

            // Write Bonus Links
            // TODO

            out << "</svg>" << std::endl; // End file
        }
        
    }

}
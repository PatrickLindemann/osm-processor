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
#include "util/misc.hpp"

using namespace model;

namespace io
{

    namespace writer
    {
        
        namespace detail
        {
            
            template <typename StreamType, typename T>
            void write_geometry(StreamType& stream, const geometry::MultiPolygon<T>& geometry)
            {
                for (const auto& polygon : geometry)
                {
                    // Add outer points (clockwise)
                    stream << "\"M ";
                    for (auto it = polygon.outer.begin(); it != polygon.outer.end(); ++it)
                    {   
                        if (it == polygon.outer.begin() + 1)
                        {
                            stream << "L ";
                        }
                        stream << it->x << " " << it->y << " ";
                    }
                    stream << "Z";
                    if (polygon.inners.size() > 0)
                    {
                        // Add inner points (counter-clockwise)
                        for (const auto& inner : polygon.inners)
                        {
                            stream << " M ";
                            for (auto it = inner.rbegin(); it != inner.rend(); ++it)
                            {   
                                if (it == inner.rbegin() + 1)
                                {
                                    stream << "L ";
                                }
                                stream << it->x << " " << it->y << " ";
                            }
                            stream << "Z";
                        }
                    }
                }
                stream << "\""; // End value
            }

        }

        /**
         * 
         */
        template <typename T>
        void write_metadata(std::string file_path, const map::Map<T>& map)
        {
            std::ofstream out{ file_path + ".json", std::ios::trunc };
            out.precision(4);

            // Write headers
            out << "{"
                << "\"name\":" << '\"' << map.name() << '\"' << ','
                << "\"created\":" << '\"' << util::get_current_iso_timestamp() << '\"' << ','
                << "\"levels\":" << '[' << util::join(map.levels()) << ']' << ',';

            // Write boundary information
            out << "\"boundaries\":" << '[';
            // Write territories
            out << "\"territories\":" << '[';
            for (const map::Territory<T>& territory : map.territories())
            {
                out << "\"id\":" << territory.id() << ','
                    << "\"name\":" << '\"' << territory.name() << '\"' << ','
                    << "\"center\":" << '['
                        << territory.center().x << ','
                        << territory.center().y
                    << ']' << ','
                    << "\"neighbors\":" << '['
                        << util::join(territory.neighbors())
                    << ']';
            }
            out << ']'; // End territories
            // Write bonuses
            out << "\"bonuses\":[";
            for (const map::Bonus<T>& bonus : map.bonuses())
            {
                out << "\"id\":" << bonus.id() << ','
                    << "\"name\":" << '\"' << bonus.name() << '\"' << ','
                    << "\"color\":" << '\"' << bonus.color() << '\"' << ','
                    << "\"armies\":" << '\"' << bonus.armies() << '\"' << ','
                    << "\"super_bonus\":" << '\"' << bonus.is_super() << '\"' << ','
                    << "\"children\":" << '[' << util::join(bonus.children()) << ']';
            }
            out << ']'; // End bonues
            out << ']'; // End boundaries

            out << "}" << std::endl; // End file
        }

        /**
         * 
         */
        template <typename T>
        void write_map(std::string file_path, const map::Map<T>& map)
        {
            std::ofstream out{ file_path + ".svg", std::ios::trunc };
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

            // Write bonuses in their order first

            // Write territories
            for (const map::Territory<T>& territory : map.territories())
            {
                // Ignore bonus links for now
                out << "<path "
                    << "id=\"Territory_" << territory.id() << "\" "
                    << "style=\"fill: blue; stroke:black; stroke-width: 1px;\" "
                    << "d=\"";
                detail::write_geometry(out, territory.geometry());
                out << "/>"; // End path
            }

            // Write Bonus Links

            out << "</svg>" << std::endl; // End file
        }

        /**
         * 
         */
        template <typename T>
        void write_preview(std::string file_path, const map::Map<T>& map)
        {
            /*
            // Draw envelopes
            for (const auto& boundary : map.boundaries())
            {
                for (const auto& poly : boundary.geometry().polygons)
                {
                    auto envelope = mapmaker::algorithm::envelope(poly);
                    out << "<rect "
                        << "id=\"" << boundary.id() << "\" "
                        << "x=\"" << envelope.min.x << "\" "
                        << "y=\"" << envelope.min.y << "\" "
                        << "width=\"" << envelope.width() << "\" "
                        << "height=\"" << envelope.height() << "\" "
                        << "fill=\"none\" " 
                        << "stroke=\"red\" " 
                        << "stroke-width=\"1\"" 
                        << "/>";
                }
                out << "<circle "
                    << "id=\"Center_" << boundary.id() << "\" "
                    << "cx=\"" << boundary.center().x << "\" "
                    << "cy=\"" << boundary.center().y << "\" "
                    << "r=\"2\" "
                    << "fill=\"black\""
                    << "/>";
                out << "<circle "
                    << "id=\"Center_Radius_" << boundary.id() << "\" "
                    << "cx=\"" << boundary.center().x << "\" "
                    << "cy=\"" << boundary.center().y << "\" "
                    << "r=\"20\" "
                    << "fill=\"none\" "
                    << "stroke=\"black\" "
                    << "stroke-width=\"1\""
                    << "/>";
            }

            // Add footers
            out << "</svg>" << std::endl;
            */      
        }
        
    }

}
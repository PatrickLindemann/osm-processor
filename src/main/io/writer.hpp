#ifndef IO_WRITER_HPP
#define IO_WRITER_HPP

#include <boost/algorithm/string/join.hpp>
#include <cfloat>
#include <cmath>
#include <string>
#include <iostream>
#include <fstream>
#include <cassert>

#include "mapmaker/model.hpp"
#include "geometry/model.hpp"

namespace io
{

    namespace writer
    {

        const void write_svg(std::string file_path, mapmaker::model::Map& map)
        {
            std::ofstream out{ file_path, std::ios::trunc };
            out.precision(4);

            if (!out.is_open())
            {
                return; // TODO Error handling
            }

            // Add headers
            out << "<svg xmlns=\"http://www.w3.org/2000/svg\" "
            << "id=\"my-svg\" "
            << "width=\"" << map.width() << "px\" "
            << "height=\"" << map.height() << "px\""
            << ">" << std::endl;

            // Add paths
            size_t id = 0;
            for (const auto& [k, t] : map.territories())
            {
                out << "<path "
                    << "id=\"Territory_" << ++id << "\" "
                    << "style=\"fill: none; stroke:black; stroke-width: 1px;\" "
                    << "d=\"";
                for (const auto& polygon : t.geometry.polygons)
                {
                    // Add outer points (clockwise)
                    out << "M ";
                    for (auto i = polygon.outer.begin(); i != polygon.outer.end(); ++i)
                    {   
                        if (i == polygon.outer.begin() + 1)
                            out << "L ";
                        out << i->x << " " << i->y << " ";
                    }
                    out << "Z";
                    if (polygon.inners.size() > 0)
                    {
                        // Add inner points (counter-clockwise)
                        for (const auto& inner : polygon.inners)
                        {
                            out << " M ";
                            for (auto i = inner.rbegin(); i != inner.rend(); ++i)
                            {   
                                if (i == inner.rbegin() + 1)
                                    out << "L ";
                                out << i->x << " " << i->y << " ";
                            }
                            out << "Z";
                        }
                    }
                }
                out << "\"/>" << std::endl;
            }

            // Add footers
            out << "</svg>" << std::endl;

        }

    }

}

#endif
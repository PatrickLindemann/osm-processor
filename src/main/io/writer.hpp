#pragma once

#include <cfloat>
#include <cmath>
#include <string>
#include <iostream>
#include <fstream>
#include <cassert>
#include <type_traits>

#include "model/map/map.hpp"
#include "model/map/boundary.hpp"
#include "util/functions.hpp"

using namespace model;

namespace io
{

    namespace writer
    {

        void write_svg(std::string file_path, const map::Map& map)
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
            << ">";

            // Add paths
            size_t id = 0;
            for (const auto& boundary : map.boundaries())
            {
                // Ignore bonus links for now
                if (boundary.type() == model::map::Boundary::BONUS)
                {
                    continue;   
                }

                out << "<path "
                    << "id=\"Territory_" << ++id << "\" "
                    << "style=\"fill: blue; stroke:black; stroke-width: 1px;\" "
                    << "d=\"";
                for (const auto& polygon : boundary.geometry().polygons)
                {
                    // Add outer points (clockwise)
                    out << "M ";
                    for (auto it = polygon.outer.begin(); it != polygon.outer.end(); ++it)
                    {   
                        if (it == polygon.outer.begin() + 1)
                        {
                            out << "L ";
                        }
                        out << it->x << " " << it->y << " ";
                    }
                    out << "Z";
                    if (polygon.inners.size() > 0)
                    {
                        // Add inner points (counter-clockwise)
                        for (const auto& inner : polygon.inners)
                        {
                            out << " M ";
                            for (auto it = inner.rbegin(); it != inner.rend(); ++it)
                            {   
                                if (it == inner.rbegin() + 1)
                                {
                                    out << "L ";
                                }
                                out << it->x << " " << it->y << " ";
                            }
                            out << "Z";
                        }
                    }
                }
                out << "\"/>";

                // Draw centerpoints
                out << "<circle cx=\"" << boundary.center().x << "\" cy=\"" << boundary.center().y << "\" r=\"20\"/>";

            }

            // Add footers
            out << "</svg>" << std::endl;

        }

    }

}
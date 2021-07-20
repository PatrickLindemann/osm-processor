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
#include "geometry/projection.hpp"
#include "geometry/algorithm.hpp"

namespace io
{

    namespace writer
    {

        const void write_svg(std::string file_path, mapmaker::model::Map& map, int32_t width, int32_t height)
        {
            std::ofstream out{ file_path, std::ios::trunc };
            out.precision(4);

            if (!out.is_open())
            {
                return; // TODO Error handling
            }

            // Calculate map bounds geometry bounds
            geometry::model::Rectangle map_bounds = { DBL_MAX, DBL_MAX, -DBL_MAX, -DBL_MAX };
            for (const auto& [k, t] : map.territories())
            {
                map_bounds.extend(geometry::algorithm::bounds(t.geometry));
            }
            // TODO how to handle bonus links?

            // Determine map width and size if set to auto
            assert(!(width == 0 && height == 0));
            if (width == 0)
            {
                width = map_bounds.width() / map_bounds.height() * height;
            }
            else if (height == 0)
            {
                height = map_bounds.height() / map_bounds.width() * width;
            }

            // Create projection to translate points to the unit interval
            geometry::projection::Interval map_interval{
                map_bounds.min.x,
                map_bounds.min.y,
                map_bounds.max.x,
                map_bounds.max.y
            };
            geometry::projection::UnitProjection projection{ map_interval };

            // Add headers
            out << "<svg xmlns=\"http://www.w3.org/2000/svg\" "
            << "id=\"my-svg\" "
            << "width=\"" << width << "px\" "
            << "height=\"" << height << "px\""
            << ">" << std::endl;

            // Add paths
            size_t id = 0;
            for (const auto& [k, t] : map.territories())
            {
                for (const auto& polygon : t.geometry.polygons)
                {
                    out << "<path "
                        << "id=\"Territory_" << ++id << "\" "
                        << "d=\"";
                    // Add outer points (clockwise)
                    out << "M ";
                    for (auto i = polygon.outer.begin(); i != polygon.outer.end(); ++i)
                    {   
                        // Project point to unit interval [0, 1]
                        geometry::model::Point p { projection.translate(i->x, i->y) };
                        // Rotate point by -90 degrees
                        p = { p.y, -p.x + 1 };
                        // Scale point to map bounds
                        p = { p.x * width, p.y * height };
                        // Write point to output
                        if (i == polygon.outer.begin() + 1)
                            out << "L ";
                        out << p.x << " " << p.y << " ";
                        std::cout << "(" << i->x << "," << i->y << ") -> (" << p.x << "," << p.y << ")" << std::endl;
                    }
                    out << "Z";
                    // Add inner points (counter-clockwise)
                    if (polygon.inners.size() > 0)
                    {
                        for (const auto& inner : polygon.inners)
                        {
                            out << "M ";
                            for (auto i = inner.rbegin(); i != inner.rend(); ++i)
                            {   
                                // Project point to unit interval [0, 1]
                                geometry::model::Point p { projection.translate(i->x, i->y) };
                                // Rotate point by -90 degrees
                                p = { p.y, -p.x + 1 };
                                // Scale point to map bounds
                                p = { p.x * width, p.y * height };
                                // Write point to output
                                if (i == inner.rbegin() + 1)
                                    out << "L ";
                                out << p.x << " " << p.y << " ";
                            }
                            out << "Z";
                        }
                    }
                    out << "\"/>" << std::endl;
                }
            }

            // Add footers
            out << "</svg>" << std::endl;

        }

    }

}

#endif
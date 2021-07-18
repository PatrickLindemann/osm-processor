#ifndef IO_WRITER_HPP
#define IO_WRITER_HPP

#include <cfloat>
#include <cmath>
#include <string>
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

            if (!out.is_open())
            {
                return; // TODO Error handling
            }

            // Calculate map bounds geometry bounds
            geometry::model::Rectangle map_bounds = { DBL_MAX, DBL_MAX, DBL_MIN, DBL_MIN };
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

            // Create projection to clamp points to the unit interval [0, 1]
            geometry::projection::Interval interval{
                map_bounds.min.x,
                map_bounds.min.y,
                map_bounds.max.x,
                map_bounds.max.y
            };
            geometry::projection::UnitIntervalProjection projection{ interval };

            // FIXME: add html
            out << "<!DOCTYPE html>"
                << "<html>"
                <<   "<head>"
                <<     "<meta charset=\"utf-8\"/>"
                <<     "<title>Test</title>"
                <<     "<link rel=\"stylesheet\" href=\"./styles.css\"/>"
                <<   "</head>"
                <<   "<body>"
                << std::endl;

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
                    for (const auto& point : polygon.outer)
                    {
                        const std::pair<double_t, double_t> txy = projection.translate(point.x, point.y);
                        out << "M " << txy.first * width << " " << txy.second * height << " ";
                    }
                    out << "Z";
                    // Add inner points (counter clockwise)
                    if (polygon.inners.size() > 0)
                    {
                        for (const auto& inner : polygon.inners)
                        {
                            for (auto i = inner.rbegin(); i != inner.rend(); ++i)
                            { 
                                const geometry::model::Point<double_t> point = *i;
                                const std::pair<double_t, double_t> txy = projection.translate(point.x, point.y);
                                out << "M " << txy.first * width << " " << txy.second * height << " ";
                            }
                        }
                        out << "Z";
                    }
                    out << "\"/>" << std::endl;
                }
            }

            // Add footers
            out << "</svg>" << std::endl;

            // FIXME:
            out <<     "<script src=\"./scripts.js\"></script>"
                <<   "</body>"
                << "</html>"
                << std::endl;

        }

    }

}

#endif
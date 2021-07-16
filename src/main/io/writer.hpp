#ifndef IO_WRITER_HPP
#define IO_WRITER_HPP

#include <string>
#include <sstream>

#include "mapmaker/model.hpp"

namespace io
{

    namespace writer
    {

        const void write_svg(std::string file_path, mapmaker::model::Map& map)
        {
            std::stringstream ss;

            // Add headers
            ss << "<svg xmlns=\"http://www.w3.org/2000/svg\""
            << "id=\"my-svg\" "
            << "width=\"" << map.width << "px\" "
            << "height=\"" << map.height << "px\""
            << ">";

            for (const mapmaker::model::Boundary& territory : map.territories)
            {
                /*
                std::vector<Point> points;
                for (const Polygon& polygon : territory.geometry())
                {
                    for (const Point& point : polygon.outer())
                    {
                        Point pp = project(point);
                    }
                }
                */
            }
        }

    }

}

#endif
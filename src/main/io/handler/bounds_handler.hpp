#pragma once

#include <algorithm>
#include <limits>
#include <map>
#include <string>
#include <cstdint>
#include <iostream>

#include <osmium/osm/node.hpp>
#include <osmium/osm/way.hpp>
#include <osmium/osm/relation.hpp>
#include <osmium/osm/location.hpp>
#include <osmium/handler.hpp>

#include "model/geometry/point.hpp"
#include "model/geometry/rectangle.hpp"
#include "util/table.hpp"

namespace io
{

    namespace handler
    {

        using namespace model;

        /**
         * A handler that determines the bounding box of of an
         * osmium object stream.
         */
        class BoundsHandler : public osmium::handler::Handler {

            /**
             * The min and max node coordinates.
             */
            double m_min_lon = DBL_MAX;
            double m_min_lat = -DBL_MAX;
            double m_max_lon = DBL_MAX;
            double m_max_lat = -DBL_MAX;

        public:

            /* Accessors */

            const geometry::Rectangle<double> bounds() const
            {
                return geometry::Rectangle<double>{
                    { m_min_lon, m_min_lat },
                    { m_max_lon, m_max_lat }
                };
            };

            /* Osmium functions */

            void node(const osmium::Node& node) noexcept {
                osmium::Location location = node.location();
                m_min_lon = std::min(m_min_lon, location.lon());
                m_min_lat = std::min(m_min_lat, location.lat());
                m_max_lon = std::max(m_max_lon, location.lon());
                m_max_lat = std::max(m_max_lat, location.lat());
            }

        };

    }

}
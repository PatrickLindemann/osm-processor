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

            std::numeric_limits<double> limits;

            /**
             * The min and max node coordinates.
             */
            double m_lon_min = limits.max();
            double m_lat_min = limits.max();
            double m_lon_max = -limits.max();
            double m_lat_max = -limits.max();

        public:

            /* Accessors */

            const geometry::Rectangle<double> bounds() const
            {
                return geometry::Rectangle<double>{
                    m_lon_min,
                    m_lat_min,
                    m_lon_max,
                    m_lat_max 
                };
            };

            /* Osmium functions */

            void node(const osmium::Node& node) noexcept {
                osmium::Location location = node.location();
                m_lon_min = std::min(m_lon_min, location.lon());
                m_lat_min = std::min(m_lat_min, location.lat());
                m_lon_max = std::max(m_lon_max, location.lon());
                m_lat_max = std::max(m_lat_max, location.lat());
            }

        };

    }

}
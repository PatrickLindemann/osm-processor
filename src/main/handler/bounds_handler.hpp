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

namespace handler
{

    using namespace model;

    /**
     * Port of https://github.com/osmcode/libosmium/blob/master/examples/osmium_count.cpp
     */
    template <typename T>
    class BoundsHandler : public osmium::handler::Handler {

        std::numeric_limits<T> m_limits;

        /**
         * The min and max node coordinates.
         */
        T m_min_lon = m_limits.max(), m_min_lat = m_limits.max();
        T m_max_lon = m_limits.lowest(), m_max_lat = m_limits.lowest();

    public:

        /* Accessors */

        const geometry::Rectangle<T> bounds() const
        {
            return geometry::Rectangle<T>{
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
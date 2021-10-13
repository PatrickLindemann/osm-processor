#pragma once

#include "model/memory/node.hpp"
#include "model/memory/types.hpp"
#include "model/memory/way.hpp"
#include "model/memory/area.hpp"
#include "model/memory/ring.hpp"
#include "model/memory/buffer.hpp"
#include "model/geometry/rectangle.hpp"
#include "model/geometry/polygon.hpp"
#include "model/geometry/multipolygon.hpp"

namespace functions
{

    using namespace model;

    /**
     * Calculates the envelope of an area.
     *
     * @param area The area
     * @param nodes The node buffer
     * @return The axis-oriented minimal bounding box that encloses the multipolygon.
     */
    template <typename T>
    geometry::Rectangle<T> envelope(
        const model::memory::Area<T>& area,
        const memory::Buffer<memory::Node<T>>& nodes
    ) {
        std::numeric_limits<T> limits;
        T min_lon = limits.max(), min_lat = limits.max() ;
        T max_lon = limits.lowest(), max_lat = limits.lowest();
        for (const memory::Ring<T>& ring : area.outer_rings())
        {
            for (const memory::object_id_type& node_ref : ring)
            {
                const memory::Node<T>& node = nodes.get(node_ref);
                min_lon = std::min(min_lon, node.lon());
                min_lat = std::min(min_lat, node.lat());
                max_lon = std::max(max_lon, node.lon());
                max_lat = std::max(max_lat, node.lat());
            }
        }
        return geometry::Rectangle<T>( { min_lon, min_lat }, { max_lon, max_lat } );
    }

    /**
     * Calculates the envelope of a polygon.
     *
     * @param polygon The polygon
     * @return The axis-oriented minimal bounding box that encloses the polygon.
     */
    template <typename T>
    geometry::Rectangle<T> envelope(const geometry::Polygon<T>& polygon)
    {
        std::numeric_limits<T> limits;
        T min_x = limits.max(), min_y = limits.max() ;
        T max_x = limits.lowest(), max_y = limits.lowest();
        for (const geometry::Point<T>& p : polygon.outer)
        {
            min_x = std::min(min_x, p.x);
            min_y = std::min(min_y, p.y);
            max_x = std::max(max_x, p.x);
            max_y = std::max(max_y, p.y);
        }
        return geometry::Rectangle<T>( { min_x, min_y }, { max_x, max_y } );
    }

    /**
     * Calculates the envelope of a multipolygon.
     *
     * @param multipolygon The multipolygon
     * @return The axis-oriented minimal bounding box that encloses the multipolygon.
     */
    template <typename T>
    geometry::Rectangle<T> envelope(const geometry::MultiPolygon<T>& multipolygon)
    {
        std::numeric_limits<T> limits;
        T min_x = limits.max(), min_y = limits.max() ;
        T max_x = limits.lowest(), max_y = limits.lowest();
        for (const geometry::Polygon<T>& polygon : multipolygon.polygons)
        {
            geometry::Rectangle<T> e = envelope(polygon);
            min_x = std::min(min_x, e.min.x);
            min_y = std::min(min_y, e.min.y);
            max_x = std::max(max_x, e.max.x);
            max_y = std::max(max_y, e.max.y);
        }
        return geometry::Rectangle<T>( { min_x, min_y }, { max_x, max_y } );
    }

}
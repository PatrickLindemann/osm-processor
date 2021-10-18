#pragma once

#include <limits>

#include "model/geometry/rectangle.hpp"
#include "model/geometry/polygon.hpp"
#include "model/geometry/multipolygon.hpp"

using namespace model;

namespace functions
{

    /**
     * Calculates the envelope of a polygon, which is the
     * axis-oriented minimal bounding box that encloses the
     * outer ring of the polygon.
     *
     * @param polygon The polygon
     * @return        The polygon bounding box
     * 
     * Time complexity: Linear
     */
    template <typename T>
    geometry::Rectangle<T> envelope(const geometry::Polygon<T>& polygon)
    {
        std::numeric_limits<T> limits;
        T min_x = limits.max();
        T min_y = limits.max();
        T max_x = -limits.max();
        T max_y = -limits.max();
        for (const geometry::Point<T>& point : polygon.outer())
        {
            min_x = std::min(min_x, point.x);
            min_y = std::min(min_y, point.y);
            max_x = std::max(max_x, point.x);
            max_y = std::max(max_y, point.y);
        }
        return geometry::Rectangle<T>{ min_x, min_y, max_x, max_y };
    }

    /**
     * Calculates the envelope of a multipolygon, which is
     * the axis-oriented minimal bounding box that encloses
     * all exclaves of the multipolygon.
     *
     * @param multipolygon The multipolygon
     * @return             The multipolygon bounding box
     * 
     * Time complexity: Linear
     */
    template <typename T>
    geometry::Rectangle<T> envelope(const geometry::MultiPolygon<T>& multipolygon)
    {
        std::numeric_limits<T> limits;
        T min_x = limits.max();
        T min_y = limits.max();
        T max_x = -limits.max();
        T max_y = -limits.max();
        for (const geometry::Polygon<T>& polygon : multipolygon.polygons())
        {
            for (const geometry::Point<T>& point : polygon.outer())
            {
                min_x = std::min(min_x, point.x);
                min_y = std::min(min_y, point.y);
                max_x = std::max(max_x, point.x);
                max_y = std::max(max_y, point.y);
            }
        }
        return geometry::Rectangle<T>{ min_x, min_y, max_x, max_y };
    }

}
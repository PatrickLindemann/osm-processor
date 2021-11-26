#pragma once

#include <limits>

#include "model/geometry/point.hpp"
#include "model/geometry/rectangle.hpp"
#include "model/geometry/ring.hpp"
#include "model/geometry/polygon.hpp"
#include "model/geometry/multipolygon.hpp"

using namespace model::geometry;

namespace functions
{

    /**
     * Calculates the envelope of a ring, which is the axis-
     * oriented minimal bounding box that encloses the ring.
     *
     * @param ring    The ring
     * @return        The bounding box of the ring
     * 
     * Time complexity: Linear
     */
    template <typename T>
    inline Rectangle<T> envelope(const Ring<T>& ring)
    {
        std::numeric_limits<T> limits;
        T min_x = limits.max();
        T min_y = limits.max();
        T max_x = -limits.max();
        T max_y = -limits.max();
        for (const Point<T>& point : ring)
        {
            min_x = std::min(min_x, point.x());
            min_y = std::min(min_y, point.y());
            max_x = std::max(max_x, point.x());
            max_y = std::max(max_y, point.y());
        }
        return Rectangle<T>{ min_x, min_y, max_x, max_y };
    }

    /**
     * Calculates the envelope of a polygon, which is the
     * axis-oriented minimal bounding box that encloses the
     * outer ring of the polygon.
     *
     * @param polygon The polygon
     * @return        The bounding box of the polygon
     * 
     * Time complexity: Linear
     */
    template <typename T>
    inline Rectangle<T> envelope(const Polygon<T>& polygon)
    {
        return envelope(polygon.outer());
    }

    /**
     * Calculates the envelope of a multipolygon, which is
     * the axis-oriented minimal bounding box that encloses
     * all exclaves of the multipolygon.
     *
     * @param multipolygon The multipolygon
     * @return             The bounding box of the multipolygon
     * 
     * Time complexity: Linear
     */
    template <typename T>
    inline Rectangle<T> envelope(const MultiPolygon<T>& multipolygon)
    {
        std::numeric_limits<T> limits;
        T min_x = limits.max();
        T min_y = limits.max();
        T max_x = -limits.max();
        T max_y = -limits.max();
        for (const Polygon<T>& polygon : multipolygon.polygons())
        {
            for (const Point<T>& point : polygon.outer())
            {
                min_x = std::min(min_x, point.x());
                min_y = std::min(min_y, point.y());
                max_x = std::max(max_x, point.x());
                max_y = std::max(max_y, point.y());
            }
        }
        return Rectangle<T>{ min_x, min_y, max_x, max_y };
    }

}
#ifndef MAPMAKER_MULTIPOLYGON_HPP
#define MAPMAKER_MULTIPOLYGON_HPP

#include <vector>

#include "model/box.hpp"
#include "model/point.hpp"
#include "model/entity.hpp"
#include "model/polygon.hpp"

#include "functions.hpp"

namespace mapmaker
{

/**
 * A MultiPolygon is a multi surface container whose elements
 * are Polygons.
 * 
 * The following assertions must be fullfilled in order to
 * create a valid MultiPolygon:
 * 
 * - A MultiPolygon is defined as topologically closed,
 * - A MultiPolygon may not have cut lines, spikes or punctures,
 * - The interiors of 2 Polygons that are elements of a
 *   MultiPolygon may not intersect,
 * - The boundaries of any 2 Polygons that are elements of a
 *   MultiPolygon may not cross and may touch at only a finite
 *   number of Points,
 * - The interior of a MultiPolygon with more than 1 Polygon
 *   is not connected,
 * - The number of connected components of the interior of a
 *   MultiPolygon is equal to the number of Polygons.
 * 
 * Adapted from https://gis.stackexchange.com/questions/225368/understanding-difference-between-polygon-and-multipolygon-for-shapefiles-in-qgis/225373
 * 
 */
class MultiPolygon : public Entity
{

    const std::vector<Polygon> m_polygons;

public:

    MultiPolygon(const std::vector<Polygon>& polygons) : m_polygons(polygons) {};

    ~MultiPolygon() {};

    const std::vector<Polygon>& polygons() const
    {
        return m_polygons;
    }

    const bool empty() const
    {
        return m_polygons.empty();
    }

    auto begin() noexcept
    {
        return m_polygons.begin();
    }

    auto end() noexcept
    {
        return m_polygons.end();
    }

    const Point center() const
    {
        // TODO
        return Point{};
    }

    const Box bounds() const
    {
        if (m_polygons.empty())
            return Box();   
        Box bounds;
        for (const Polygon& polygon : m_polygons)
            bounds.extend(polygon.bounds());
        return bounds;
    }

    const Polygon& operator[](const size_t index)
    {
        return m_polygons[index];
    }

};

}

#endif
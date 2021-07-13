#ifndef MAPMAKER_POLYGON_HPP
#define MAPMAKER_POLYGON_HPP

#include <cfloat>
#include <utility>
#include <vector>
#include <iterator>
#include <initializer_list>

#include "model/box.hpp"
#include "model/entity.hpp"
#include "model/point.hpp"
#include "model/ring.hpp"

#include "functions.hpp"

namespace mapmaker
{

/**
 * A Polygon is a planar surface defined by 1 exterior and 0
 * or more interior boundaries (rings). Each interior boundary
 * defines a polygon.
 * 
 * The following assertions must be fullfilled in order to
 * create a valid Polygon:
 * 
 * - Polygons are topologically closed,
 * - The boundary of a Polygon consists of a set of rings that
 *   make up its exterior and interior boundaries,
 * - No two rings in the boundary cross and the rings in the
 *   boundary may intersect at a point but only as a tangent,
 * - A Polygon may not have cut lines, spikes or punctures,
 * - The interior of every Polygon is a connected point set,
 * - The exterior of every Polygon with 1 or more holes is not
 *   connected. Each hole defines a connected component of the
 *   exterior.
 * 
 * Adapted from https://gis.stackexchange.com/questions/225368/understanding-difference-between-polygon-and-multipolygon-for-shapefiles-in-qgis/225373
 * 
 */
class Polygon : public Entity
{

    const Ring m_outer;

    const std::vector<Ring> m_inners;

public:

    Polygon(const Ring& outer_ring) : m_outer(outer_ring) {};

    Polygon(const Ring& outer_ring, const std::vector<Ring>& inner_rings)
    : m_outer(outer_ring), m_inners(inner_rings) {};

    Polygon(const Ring& outer_ring, const std::initializer_list<Ring>& inner_rings)
    : m_outer(outer_ring), m_inners(inner_rings) {};

    ~Polygon() {};

    const Ring& outer() const
    {
        return m_outer;
    }

    const std::vector<Ring>& inners() const
    {
        return m_inners;
    }

    const Point center() const
    {
        return functions::calc_center(m_outer.points());
    }

    const Box bounds() const
    {
        return functions::calc_bounds(m_outer.points());
    }

};

}

#endif
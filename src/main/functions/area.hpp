#pragma once

#include "model/geometry/point.hpp"
#include "model/geometry/rectangle.hpp"
#include "model/geometry/ring.hpp"
#include "model/geometry/polygon.hpp"
#include "model/geometry/multipolygon.hpp"

using namespace model::geometry;

namespace functions
{

    /**
     * Calculate the surface area of a rectangle.
     * 
     * @param rectangle The rectangle
     * @returns         The area of the rectangle
     * 
     * Time complexity: Constant
     */
    template <typename T>
    inline double area(const Rectangle<T>& rectangle)
    {
        return rectangle.width() * rectangle.height();
    }

    /**
     * Calculate the surface area of a ring using the shoelace formula.
     * 
     * For more information and proof of this formula, refer
     * to https://en.wikipedia.org/wiki/Shoelace_formula
     * 
     * @param ring  The ring
     * @returns     The area of the ring
     * 
     * Time complexity: Linear
     */
    template <typename T>
    inline double area(const Ring<T>& ring)
    {
        double l = 0.0;
        double r = 0.0;

        for (std::size_t i = 0; i < ring.size() - 1; i++)
        {
            const Point<T>& p1 = ring.at(i);
            const Point<T>& p2 = ring.at(i + 1);
            l += p1.x() * p2.y();
            r += p2.x() * p1.y();
        }

        return 0.5 * (l - r);
    }

    /**
     * Calculate the surface area of a polygon.
     * 
     * @param polygon The polygon
     * @returns       The area of the polygon
     * 
     * Time complexity: Linear
     */
    template <typename T>
    inline double area(const Polygon<T>& polygon)
    {
        double a = area(polygon.outer());
        for (const Ring<T>& inner : polygon.inners())
        {
            a += area(inner);
        }
        return a;
    }

    /**
     * Calculate the surface area of a multipolygon.
     * 
     * @param multipolygon The multipolygon
     * @returns            The area of the multipolygon
     * 
     * Time complexity: Linear
     */
    template <typename T>
    inline double area(const MultiPolygon<T>& multipolygon)
    {
        double a = 0.0;
        for (const Polygon<T>& polygon : multipolygon.polygons())
        {
            a += area(polygon);
        }
        return a;
    }

}
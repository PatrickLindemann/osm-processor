#pragma once

#include "model/geometry/point.hpp"
#include "model/geometry/rectangle.hpp"
#include "model/geometry/ring.hpp"
#include "model/geometry/polygon.hpp"
#include "model/geometry/multipolygon.hpp"

#include "functions/area.hpp"

using namespace model::geometry;

namespace functions
{

    /**
     * Calculate the centerpoint of a rectangle.
     *
     * @param rectangle The rectangle
     * @return A pair of the center point and its distance 
     */
    template <typename T>
    inline Point<T> center(const Rectangle<T>& rectangle)
    {  
        return Point<T>{
            rectangle.min().x() + rectangle.width() / 2,
            rectangle.min().y() + rectangle.height() / 2
        };
    }

    /**
     * Calculate the center point of a ring, which is the point
     * the weighted sum of all points in the ring.
     * 
     * Note: This algorithm does not provide the (optimal) point of
     * isolation, but the approximation of it is enough for our case.
     *
     * @param ring The ring
     * @return     The center point of the ring
     */
    template <typename T>
    inline Point<T> center(const Ring<T>& ring)
    {
        Point<T> c{ 0, 0 };
        double a = 0.0;
        double f = 0.0;

        const Point<T> first = ring.front();
        for (std::size_t i = 0; i < ring.size() - 1; i++)
        {
            const Point<T>& p1 = ring.at(i);
            const Point<T>& p2 = ring.at(i + 1);
            f = (p1.x() - first.x()) * (p2.y() - first.y())
              - (p1.y() - first.y()) * (p2.x() - first.x());
            a += f;
            c.x() += (p1.x() + p2.x() - 2 * first.x()) * f;
            c.y() += (p1.y() + p2.y() - 2 * first.y()) * f;
        }

        if (a != 0)
        {
            c /= a * 3;
        }
        c += first;

        return c;
    }

    /**
     * Calculate the center point of a polygon, which is the point
     * the weighted sum of all center points of each ring weighted
     * with the respective ring area.
     *
     * @param polygon The polygon.
     * @return        The center point of the polygon.
     */
    template <typename T>
    inline Point<T> center(const Polygon<T>& polygon)
    {
        Point<T> c = center(polygon.outer());

        if (!polygon.inners().empty())
        {
            double a = area(polygon.outer());
            c *= a;

            for (const Ring<T>& inner : polygon.inners())
            {
                Point<T> c_i = center(inner);
                double a_i = area(inner);
                c += c_i * a_i;
                a += a_i;
            }

            if (a != 0)
            {
                c /= a;
            }
        }

        return c;
    }

    /**
     * Calculate the center point of a polygon, which is the point that has
     * the maximum distance inside any of the polygons (point of isolation).
     *
     * @param multipolygon The multipolygon.
     * @return The center point of the multipolygon.
     */
    template <typename T>
    inline Point<T> center(const MultiPolygon<T>& multipolygon)
    {
        Point<T> c{ 0.0, 0.0 };
        double a = 0.0;

        for (const Polygon<T>& polygon : multipolygon.polygons())
        {
            Point<T> c_p = center(polygon);
            double a_p = area(polygon);
            c += c_p * a_p;
            a += a_p;
        }

        if (a != 0)
        {
            c /= a;
        }

        return c;
    }

}
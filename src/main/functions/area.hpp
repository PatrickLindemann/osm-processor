#pragma once

#include <algorithm>

#include "model/geometry/point.hpp"
#include "model/geometry/rectangle.hpp"
#include "model/geometry/circle.hpp"
#include "model/geometry/polygon.hpp"
#include "model/geometry/multipolygon.hpp"

namespace functions
{

    using namespace model::geometry;

    /**
     * Calculate the area of a rectangle.
     * 
     * @param rectangle The rectangle
     * @returns         The area of the rectangle
     * 
     * Time complexity: Constant
     */
    template <typename T>
    double area(const Rectangle<T>& rectangle)
    {
        return rectangle.width() * rectangle.height();
    }

    /**
     * Calculate the area of a circle.
     * 
     * @param circle The circle
     * @returns      The area of the circle
     * 
     * Time complexity: Constant
     */
    template <typename T>
    double area(const Circle<T>& circle)
    {
        return M_PI * std::pow(circle.radius, 2);
    }

    /**
     * Calculate the area of a ring with consecutive points
     * in counter-clockwise order.
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
    double area(const Ring<T>& ring)
    {
        double left_sum = 0.0;
        double right_sum = 0.0;

        for (size_t i = 0, j = ring.size() - 1; j < ring.size(); j = i + 1)
        {
            left_sum += ring.at(i).x * ring.at(j).y;
            right_sum += ring.at(j).x * ring.at(i).y;
        }

        return 0.5 * std::abs(left_sum - right_sum);
    }

    /**
     * Calculate the area of a polygon with rings of 
     * consecutive points in counter-clockwise order.
     * 
     * @param polygon The polygon
     * @returns       The area of the polygon
     * 
     * Time complexity: Linear
     */
    template <typename T>
    double area(const Polygon<T>& polygon)
    {
        double outer_area = area(polygon.outer);
        double inner_area = 0.0;
        for (const Ring<T>& inner : polygon.inners)
        {
            inner_area += area(inner);
        }
        return std::max(0.0, outer_area - inner_area);
    }

    /**
     * Calculate the area of a multipolygon.
     * 
     * @param multipolygon The multipolygon
     * @returns            The area of the multipolygon
     * 
     * Time complexity: Linear
     */
    template <typename T>
    double area(const MultiPolygon<T>& multipolygon)
    {
        double area_sum = 0.0;
        for (const Polygon<T>& polygon : multipolygon)
        {
            area_sum += area(polygon);
        }
        return area_sum;
    }

}
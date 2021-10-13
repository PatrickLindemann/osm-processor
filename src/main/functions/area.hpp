#pragma once

#include "model/geometry/point.hpp"
#include "model/geometry/rectangle.hpp"
#include "model/geometry/circle.hpp"
#include "model/geometry/polygon.hpp"
#include "model/geometry/multipolygon.hpp"

using namespace model;

namespace functions
{

    /**
     * 
     */
    template <typename T>
    double area(const geometry::Rectangle<T>& rectangle)
    {
        return rectangle.width() * rectangle.height();
    }

    /**
     * 
     */
    template <typename T>
    double area(const geometry::Circle<T>& circle)
    {
        return M_PI * std::pow(circle.radius, 2);
    }

    /**
     * 
     */
    template <typename T>
    double area(const geometry::Polygon<T>& polygon)
    {
        return 0.0;
    }

    /**
     * 
     */
    template <typename T>
    double area(const geometry::MultiPolygon<T>& multipolygon)
    {
        return 0.0;
    }

}
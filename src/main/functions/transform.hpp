#pragma once

#pragma once

#include <cmath>

#include "functions/util.hpp"
#include "model/geometry/point.hpp"

using namespace model;


namespace functions
{

    /**
     * The projection base class.
     * A (two-dimensional) transformation is a function X * Y -> X' * Y' that maps
     * two values (x, y) from the domain to the (x', y') in the image.
     */
    template <typename T>
    class Transformation
    {
    public:

        /**
         * Transform a pair of values.
         * 
         * @param x The x value
         * @param y The y value
         */
        virtual geometry::Point<T> transform(T x, T y) const = 0;

    };

    /**
     * A transformations that sclaes values with the factors (s_x, s_y)
     * (x, y) -> (s_x * x, s_y * y)
     */
    template <typename T>
    class ScaleTranformation : public Transformation<T>
    {

        T m_sx;
        T m_sy;

    public:

        ScaleTranformation(T sx, T sy) : m_sx(sx), m_sy(sy) {}

        /**
         * Project a pair of values to themselves.
         * 
         * @param x The x value
         * @param y The y value
         */
        geometry::Point<T> transform(T x, T y) const override
        {
            return geometry::Point<T>{ m_sx * x, m_sy *y };
        }

    };

}
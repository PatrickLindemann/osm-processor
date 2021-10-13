#pragma once

#include <cmath>

#include "model/geometry/point.hpp"

namespace functions
{

    using namespace model;

    const double HALF_C = M_PI / 180.0;
    const double TWO_C = 180.0 / M_PI ; 

    /**
     * Convert an angle in degrees to radians.
     * 
     * @param degrees The angle in degrees
     */
    template <typename T>
    inline T radians(T degrees)
    {
        return degrees * HALF_C;
    } 

    /**
     * Convert an angle in radians to degrees.
     * 
     * @param radians The angle in radians
     */
    template <typename T>
    inline T degrees(T radians)
    {
        return radians * TWO_C;
    }

    /**
     * Calculate the dot product of two points
     * 
     * @param p The first point
     * @param q The second point
     * @return The dot product of p and q
     * 
     * Time complexity: Constant
     */
    template <typename T>
    inline double dot(
        const geometry::Point<T>& p,
        const geometry::Point<T>& q
    ) {
        return p.x * q.x + p.y * q.y;
    }
  
    /**
     * Normalizes a value by projecting it to an interval [lower, upper]
     * 
     * @param value The value that will be normalized
     * @param lower The lower bound of the target interval
     * @param upper The upper bound of the target interval
     */
    template <typename T>
    inline T normalize(T value, T lower, T upper)
    {
        if (value < lower)
        {
            value += std::abs(upper - lower);
        }
        else if (value > upper)
        {
            value += std::abs(upper - lower);
        }
        return value;
    }

}
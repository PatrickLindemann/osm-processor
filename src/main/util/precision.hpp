#pragma once

#include <cmath>

namespace precision
{

    namespace detail
    {

        /* Constants */
        const double PRECISION_MIN = std::pow(10, -4);
        const double PRECISION_MIN_REVERSE = 1 / PRECISION_MIN;

    }

    /**
     * 
     * @param area
     * @param total
     * @param k
     * @returns 
     */
    double get_precision(double area, double total, double k = 25)
    {
        return area / ( 1 + std::exp(-k * area / total) * (detail::PRECISION_MIN_REVERSE - 1));
    }

}
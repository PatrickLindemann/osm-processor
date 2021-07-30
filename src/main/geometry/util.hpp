#ifndef GEOMETRY_UTIL_HPP
#define GEOMETRY_UTIL_HPP

#include <cmath>

namespace geometry
{
    namespace util
    {

        /* Constants */

        const double_t QUARTER_PI = M_PI / 4.0;
        const double_t HALF_PI = M_PI / 2.0;
        const double_t TWO_PI = M_PI * 2.0;
        const double_t TWO_PI_INVERSE = 1 / (M_PI * 2.0);

        const double_t HALF_C = M_PI / 180.0;
        const double_t TWO_C = 180.0 / M_PI ; 

        /* Methods */

        /**
         * Convert an angle in degrees to radians
         * 
         * @param degrees The angle in degrees
         */
        inline double_t radians(double_t degrees)
        {
            return degrees * HALF_C;
        } 

        /**
         * Convert an angle in radians to degrees
         * 
         * @param radians The angle in radians
         */
        inline double_t degrees(double_t radians)
        {
            return radians * TWO_C;
        }

        /**
         * Normalizes an angle in radians so that it is within the interval
         * [0; 2 * PI].
         * 
         * @param radians The angle in radians
         */
        inline double_t normalize_radians(double_t radians)
        {
            return std::fmod(std::fmod(radians, TWO_PI) + TWO_PI, TWO_PI);
        }

        /**
         * Normalizes an angle in degrees so that it is within the interval
         * [0; 360].
         * 
         * @param degrees The angle in degrees
         */
        inline double_t normalize_degrees(double_t degrees)
        {
            return std::fmod(std::fmod(degrees, 360.0) + 360.0, 360.0);
        }

    }
}

#endif
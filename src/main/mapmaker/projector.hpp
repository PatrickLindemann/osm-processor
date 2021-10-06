#pragma once

#include <cmath>

#include "model/geometry/point.hpp"

using namespace model;

namespace mapmaker
{

    namespace projector
    {

        const double QUARTER_PI = M_PI / 4.0;
        const double HALF_C = M_PI / 180.0;
        const double TWO_C = 180.0 / M_PI ; 

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
         * The projector base class.
         * A (two-dimensional) projector is a function X * Y -> X' * Y' that maps
         * two values (x, y) from the domain to the (x', y') in the image.
         */
        template <typename T>
        class Projector
        {
        public:

            /**
             * Project a pair of values.
             * 
             * @param x The x value
             * @param y The y value
             */
            virtual geometry::Point<T> project(T x, T y) const = 0;

        };

        /**
         * A projector that maps values using the identity function.
         * [x, y] -> [x, y]
         */
        template <typename T>
        class IdentityProjector : virtual public Projector<T>
        {
        public:

            /**
             * Project a pair of values to themselves.
             * 
             * @param x The x value
             * @param y The y value
             */
            geometry::Point<T> project(T x, T y) const override
            {
                return geometry::Point<T>{ x, y };
            }

        };

        /**
         * A projector that maps values from the source interval
         * [x_min, x_max] * [y_min, y_max] to the target interval
         * [x'_min, x'_max] * [y'_min, y'_max].
         */
        template <typename T>
        class IntervalProjector : public Projector<T>
        {
            
            /* Members */

            std::pair<T, T> m_source_x, m_source_y;
            T diff_source_x, diff_source_y;

            std::pair<T, T> m_target_x, m_target_y;
            T diff_target_x, diff_target_y;

        public:

            /**
             * Create the IntervalProjector.
             * 
             * @param source_x The source interval [x_min, x_max]
             * @param source_y The source interval [y_min, y_max]
             * @param target_x The target interval [x'_min, x'_max]
             * @param target_y The target interval [y'_min, y'_max]
             */
            IntervalProjector(
                const std::pair<T, T> source_x,
                const std::pair<T, T> source_y,
                const std::pair<T, T> target_x,
                const std::pair<T, T> target_y
            ) {
                this->m_source_x = source_x;
                this->m_source_y = source_y;
                this->m_target_x = target_x;
                this->m_target_y = target_y;
                this->diff_source_x = std::abs(source_x.second - source_x.first);
                this->diff_source_y = std::abs(source_y.second - source_y.first);
                this->diff_target_x = std::abs(target_x.second - target_x.first);
                this->diff_target_y = std::abs(target_y.second - target_y.first);
            };

            /**
             * Project a pair of values from the specified source to
             * to target interval.
             * 
             * @param x The x value
             * @param y The y value
             */
            geometry::Point<T> project(T x, T y) const override
            {
                T tx = m_target_x.first + (diff_target_x / diff_source_x) * (x - m_source_x.first);
                T ty = m_target_y.first + (diff_target_y / diff_source_y) * (y - m_source_y.first);
                return geometry::Point<T>{ tx, ty };
            }

        };

        /**
         * A projector that maps values from the source interval
         * [x_min, x_max] * [y_min, y_max] to the unit interval
         * [0, 1] * [0, 1].
         */
        template <typename T>
        class UnitProjector : public IntervalProjector<T>
        {
        public:

            /**
             * Create the UnitProjector.
             * 
             * @param source_x The source interval [x_min, x_max]
             * @param source_y The source interval [y_min, y_max]
             */
            UnitProjector(const std::pair<T, T> source_x, const std::pair<T, T> source_y)
            : IntervalProjector<T>(
                source_x,
                source_y,
                std::make_pair(0.0, 1.0),
                std::make_pair(0.0, 1.0)
            ) {};

            using IntervalProjector<T>::project;

        };

        /**
         * A projector that maps values from the source interval
         * [x_min, x_max] * [y_min, y_max] to the symmetric interval
         * [-1, 1] * [-1, 1].
         */
        template <typename T>
        class SymmetricProjector : public IntervalProjector<T>
        {
        public:

            /**
             * Create the SymmetricProjector.
             * 
             * @param source_x The source interval [x_min, x_max]
             * @param source_y The source interval [y_min, y_max]
             */
            SymmetricProjector(const std::pair<T, T> source_x, const std::pair<T, T> source_y)
            : IntervalProjector<T>(
                source_x,
                source_y,
                std::make_pair(-1.0, 1.0),
                std::make_pair(-1.0, 1.0)
            ) {};

            using IntervalProjector<T>::project;

        };

        /**
         * A projector that maps degree values from the interval
         * [-180, 180] * [-90, 90] to the radian interval
         * [-PI, PI] * [-PI/2, PI/2].
         */
        template <typename T>
        class RadianProjector : public Projector<T>
        {
        public:

            /**
             * Project a pair of degree values to radian values.
             * 
             * @param x The x degree value
             * @param y The y degree value
             */
            geometry::Point<T> project(T x, T y) const override
            {
                return geometry::Point<T>{ radians(x), radians(y) };
            }

        };

        /**
         * A projector that maps radian values from the interval
         * [-PI, PI] * [-PI/2, PI/2] to the degree interval
         * [-180, 180] * [-90, 90].
         */
        template <typename T>
        class DegreeProjector : public Projector<T>
        {
        public:

            /**
             * Project a pair of degree values to radian values.
             * 
             * @param x The x radian value
             * @param y The y radian value
             */
            geometry::Point<T> project(T x, T y) const override
            {
                return geometry::Point<T>{ degrees(x), degrees(y) };
            }

        };

        /**
         * A projector that maps radian earth coordinates from the interval
         * [-PI, PI] * [-PI/2, PI/2] to the target interval
         * [0, 1] * [0, 1]
         * 
         * This projection is true to angles (conformal), while not being true to 
         * scale (non-equidistant) or areas (not equal-area).
         * 
         * https://en.wikipedia.org/wiki/Mercator_projection
         */
        template <typename T>
        class MercatorProjector : public Projector<T>
        {
        
            T m_center;
        
        public:

            /**
             * Create the MercatorProjector.
             * 
             * @param center The central meridian (in longitudes).
             *  The default value 0 specifies the meridian with latitude 0, which crosses
             *  the city of Greenwhich, United Kingdom.
             */
            MercatorProjector(T center = T(0))
            {
                this->m_center = center;
            }

            /**
             * Project a pair of geographic coordinates with the mercator
             * projection.
             * 
             * @param x The x value (maps to longitude)
             * @param y The y value (maps to latitude)
             */
            geometry::Point<T> project(T x, T y) const override
            {   
                T tx = normalize(x - m_center, -M_PI, M_PI);
                T ty = std::log(std::tan(QUARTER_PI + y / 2));
                return geometry::Point<T>{ tx, ty };
            }

        };

        /**
         * A projector that projects radian earth coordinates from the interval
         * [-PI, PI] * [-PI/2, PI/2] to the target interval
         * [0, 1] * [0, 1]
         * 
         * This projection is true to area (equal-area) while not being true to
         * angle (non-conformal) or scale (non-equidistant).
         * 
         */
        template <typename T>
        class CylindricalEqualAreaProjector : virtual public Projector<T>
        {

            T m_center;
            T m_parallel;

        public:

           /**
             * Create the CylindricalEqualAreaProjector.
             * 
             * @param center The central meridian (in longitudes).
             *  The default value 0 specifies the meridian with latitude 0, which crosses
             *  the city of Greenwhich, United Kingdom.
             * @param center The standard parallel (in latitudes).
             *  The default value 0 specifies the equator.
             */
            CylindricalEqualAreaProjector(T center = T(0), T parallel = T(0))
            {
                this->m_center = center;
                this->m_parallel = parallel;
            }

            /**
             * Project a pair of geographic coordinates with the cylindrical equal-area
             * projection.
             * 
             * @param x The x value (maps to longitude)
             * @param y The y value (maps to latitude)
             */
            geometry::Point<T> project(T x, T y) const override
            {   
                double cos_p = std::cos(m_parallel);
                T tx = normalize(x - m_center, -M_PI, M_PI) * cos_p;
                T ty = std::sin(y) / cos_p;
                return geometry::Point<T>{ tx, ty };
            }

        };

    }

}
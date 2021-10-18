#pragma once

#include <cmath>

#include "functions/util.hpp"
#include "model/geometry/point.hpp"

using namespace model;

namespace functions
{

    using namespace model::geometry;

    const double QUARTER_PI = M_PI / 4.0;

    /**
     * The projection base class.
     * A (two-dimensional) projection is a function X * Y -> X' * Y' that maps
     * two values (x, y) from the domain to the (x', y') in the image.
     */
    template <typename T>
    class Projection
    {
    public:

        /**
         * Project a pair of values.
         * 
         * @param x The x value
         * @param y The y value
         */
        virtual Point<T> project(T x, T y) const = 0;

    };

    /**
     * A projection that maps values using the identity function.
     * [x, y] -> [x, y]
     */
    template <typename T>
    class IdentityProjection : virtual public Projection<T>
    {
    public:

        /**
         * Project a pair of values to themselves.
         * 
         * @param x The x value
         * @param y The y value
         */
        Point<T> project(T x, T y) const override
        {
            return Point<T>{ x, y };
        }

    };

    /**
     * A projection that maps values from the source interval
     * [x_min, x_max] * [y_min, y_max] to the target interval
     * [x'_min, x'_max] * [y'_min, y'_max].
     */
    template <typename T>
    class IntervalProjection : public Projection<T>
    {
        
        /* Members */

        std::pair<T, T> m_source_x, m_source_y;
        T diff_source_x, diff_source_y;

        std::pair<T, T> m_target_x, m_target_y;
        T diff_target_x, diff_target_y;

    public:

        /**
         * Create the IntervalProjection.
         * 
         * @param source_x The source interval [x_min, x_max]
         * @param source_y The source interval [y_min, y_max]
         * @param target_x The target interval [x'_min, x'_max]
         * @param target_y The target interval [y'_min, y'_max]
         */
        IntervalProjection(
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
        Point<T> project(T x, T y) const override
        {
            T tx = m_target_x.first + (diff_target_x / diff_source_x) * (x - m_source_x.first);
            T ty = m_target_y.first + (diff_target_y / diff_source_y) * (y - m_source_y.first);
            return Point<T>{ tx, ty };
        }

    };

    /**
     * A projection that maps values from the source interval
     * [x_min, x_max] * [y_min, y_max] to the unit interval
     * [0, 1] * [0, 1].
     */
    template <typename T>
    class UnitProjection : public IntervalProjection<T>
    {
    public:

        /**
         * Create the UnitProjection.
         * 
         * @param source_x The source interval [x_min, x_max]
         * @param source_y The source interval [y_min, y_max]
         */
        UnitProjection(const std::pair<T, T> source_x, const std::pair<T, T> source_y)
        : IntervalProjection<T>(
            source_x,
            source_y,
            std::make_pair(0.0, 1.0),
            std::make_pair(0.0, 1.0)
        ) {};

        using IntervalProjection<T>::project;

    };

    /**
     * A projection that maps values from the source interval
     * [x_min, x_max] * [y_min, y_max] to the symmetric interval
     * [-1, 1] * [-1, 1].
     */
    template <typename T>
    class SymmetricProjection : public IntervalProjection<T>
    {
    public:

        /**
         * Create the SymmetricProjection.
         * 
         * @param source_x The source interval [x_min, x_max]
         * @param source_y The source interval [y_min, y_max]
         */
        SymmetricProjection(const std::pair<T, T> source_x, const std::pair<T, T> source_y)
        : IntervalProjection<T>(
            source_x,
            source_y,
            std::make_pair(-1.0, 1.0),
            std::make_pair(-1.0, 1.0)
        ) {};

        using IntervalProjection<T>::project;

    };

    /**
     * A projection that maps degree values from the interval
     * [-180, 180] * [-90, 90] to the radian interval
     * [-PI, PI] * [-PI/2, PI/2].
     */
    template <typename T>
    class RadianProjection : public Projection<T>
    {
    public:

        /**
         * Project a pair of degree values to radian values.
         * 
         * @param x The x degree value
         * @param y The y degree value
         */
        Point<T> project(T x, T y) const override
        {
            return Point<T>{ radians(x), radians(y) };
        }

    };

    /**
     * A projection that maps radian values from the interval
     * [-PI, PI] * [-PI/2, PI/2] to the degree interval
     * [-180, 180] * [-90, 90].
     */
    template <typename T>
    class DegreeProjection : public Projection<T>
    {
    public:

        /**
         * Project a pair of degree values to radian values.
         * 
         * @param x The x radian value
         * @param y The y radian value
         */
        Point<T> project(T x, T y) const override
        {
            return Point<T>{ degrees(x), degrees(y) };
        }

    };

    /**
     * A projection that maps radian earth coordinates from the interval
     * [-PI, PI] * [-PI/2, PI/2] to the target interval
     * [0, 1] * [0, 1]
     * 
     * This projection is true to angles (conformal), while not being true to 
     * scale (non-equidistant) or areas (not equal-area).
     * 
     * https://en.wikipedia.org/wiki/Mercator_projection
     */
    template <typename T>
    class MercatorProjection : public Projection<T>
    {
    
        T m_center;
    
    public:

        /**
         * Create the MercatorProjection.
         * 
         * @param center The central meridian (in longitudes).
         *  The default value 0 specifies the meridian with latitude 0, which crosses
         *  the city of Greenwhich, United Kingdom.
         */
        MercatorProjection(T center = T(0))
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
        Point<T> project(T x, T y) const override
        {   
            T tx = clamp(x - m_center, -M_PI, M_PI);
            T ty = std::log(std::tan(QUARTER_PI + y / 2));
            return Point<T>{ tx, ty };
        }

    };

    /**
     * A projection that projects radian earth coordinates from the interval
     * [-PI, PI] * [-PI/2, PI/2] to the target interval
     * [0, 1] * [0, 1]
     * 
     * This projection is true to area (equal-area) while not being true to
     * angle (non-conformal) or scale (non-equidistant).
     * 
     */
    template <typename T>
    class CylindricalEqualAreaProjection : virtual public Projection<T>
    {

        T m_center;
        T m_parallel;

    public:

        /**
         * Create the CylindricalEqualAreaProjection.
         * 
         * @param center The central meridian (in longitudes).
         *  The default value 0 specifies the meridian with latitude 0, which crosses
         *  the city of Greenwhich, United Kingdom.
         * @param center The standard parallel (in latitudes).
         *  The default value 0 specifies the equator.
         */
        CylindricalEqualAreaProjection(T center = T(0), T parallel = T(0))
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
        Point<T> project(T x, T y) const override
        {   
            double cos_p = std::cos(m_parallel);
            T tx = clamp(x - m_center, -M_PI, M_PI) * cos_p;
            T ty = std::sin(y) / cos_p;
            return Point<T>{ tx, ty };
        }

    };

}
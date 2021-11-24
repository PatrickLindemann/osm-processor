#pragma once

#include <utility>

#include "functions/util.hpp"

namespace functions
{

    using namespace model;

    /* Types */

    template <typename T>
    using Interval = std::pair<T, T>;

    /* Constants */

    const double QUARTER_PI = M_PI / 4.0;

    /* Base Class */

    /**
     * The projection base class.
     * A (two-dimensional) transformation is a function X * Y -> X' * Y' that maps
     * two values (x, y) from the domain to the (x', y') in the image.
     */
    template <typename T>
    class Transformation
    {
    public:

        Transformation() {}

        /**
         * Transform a pair of values.
         * 
         * @param x The x value
         * @param y The y value
         */
        virtual void transform(T& x, T& y) const = 0;

    };

    /* Transformations */

    /**
     * A transformation that maps degree values from the interval
     * [-180, 180] * [-90, 90] to the radian interval
     * [-PI, PI] * [-PI/2, PI/2].
     */
    template <typename T>
    class RadianTransformation : public Transformation<T>
    {
    public:

        using Transformation<T>::Transformation;

        /**
         * Transform a pair of degree values to radian values.
         *
         * @param x The x degree value
         * @param y The y degree value
         */
        void transform(T& x, T& y) const override
        {
            x = radians(x);
            y = radians(y);
        }

    };  

    /**
     * A transformation that maps radian values from the interval
     * [-PI, PI] * [-PI/2, PI/2] to the degree interval
     * [-180, 180] * [-90, 90].
     */
    template <typename T>
    class DegreeTransformation : public Transformation<T>
    {
    public:

        using Transformation<T>::Transformation;

        /**
         * Transform a pair of degree values to radian values.
         *
         * @param x The x radian value
         * @param y The y radian value
         */
        void transform(T& x, T& y) const override
        {
            x = degrees(x);
            y = degrees(y);
        }

    };

    /**
     * A transformation that scales values with the factors (s_x, s_y)
     * (x, y) -> (s_x * x, s_y * y)
     */
    template <typename T>
    class ScaleTransformation : public Transformation<T>
    {
    protected:

        T m_sx, m_sy;

    public:

        ScaleTransformation(T scale_x, T scale_y) : m_sx(scale_x), m_sy(scale_y) {}

        /**
         * Scale a pair of values with the specified scaling parameters.
         * 
         * @param x The x value
         * @param y The y value
         */
        void transform(T& x, T& y) const override
        {
            x *= m_sx;
            y *= m_sy;
        }

    };

    /**
     *
     */
    template <typename T>
    class MirrorTransformation : public Transformation<T>
    {
    protected:

        bool m_mx, m_my;

    public:

        MirrorTransformation(bool mirror_x, bool mirror_y) : m_mx(mirror_x), m_my(mirror_y) {}

        /**
         * Scale a pair of values with the specified scaling parameters.
         *
         * @param x The x value
         * @param y The y value
         */
        void transform(T& x, T& y) const override
        {
            if (m_mx) x = -x + 1;
            if (m_my) y = -y + 1;
        }

    };

    /**
     * A transformation that maps values from the source interval
     * [x_min, x_max] * [y_min, y_max] to the target interval
     * [x'_min, x'_max] * [y'_min, y'_max].
     */
    template <typename T>
    class IntervalTransformation : public Transformation<T>
    {
    protected:

       /**
        * The source and target intervals
        */
        Interval<T> m_sx, m_sy, m_tx, m_ty;

       /**
        * The quotients of the differences between each source and target
        * interval
        */
        T m_quotx, m_quoty;

    public:

        /**
         * Create the IntervalTransformation.
         *
         * @param source_x The source interval [x_min, x_max]
         * @param source_y The source interval [y_min, y_max]
         * @param target_x The target interval [x'_min, x'_max]
         * @param target_y The target interval [y'_min, y'_max]
         */
        IntervalTransformation(Interval<T> source_x, Interval<T> source_y, Interval<T> target_x, Interval<T> target_y)
        : m_sx(source_x), m_sy(source_y), m_tx(target_x), m_ty(target_y)
        {
            m_quotx = std::abs(m_tx.second - m_tx.first) / std::abs(m_sx.second - m_sx.first);
            m_quoty = std::abs(m_ty.second - m_ty.first) / std::abs(m_sy.second - m_sy.first);
        };

        /**
         * Transform a pair of values from the specified source to
         * to target interval.
         *
         * @param x The x value
         * @param y The y value
         */
        void transform(T& x, T& y) const override
        {
            x = m_tx.first + m_quotx * (x - m_sx.first);
            y = m_ty.first + m_quoty * (y - m_sy.first);
        }

    };

    /**
     * A transformation that maps values from the source interval
     * [x_min, x_max] * [y_min, y_max] to the unit interval
     * [0, 1] * [0, 1].
     */
    template <typename T>
    class UnitTransformation : public IntervalTransformation<T>
    {
    public:

        /**
         * Create the UnitTransformation.
         *
         * @param source_x The source interval [x_min, x_max]
         * @param source_y The source interval [y_min, y_max]
         */
        UnitTransformation(Interval<T> source_x, Interval<T> source_y)
        : IntervalTransformation<T>(source_x, source_y, Interval<T>(0, 1), Interval<T>(0, 1)) {}

        using IntervalTransformation<T>::transform;

    };

    /**
     * A transformation that maps values from the source interval
     * [x_min, x_max] * [y_min, y_max] to the symmetric interval
     * [-1, 1] * [-1, 1].
     */
    template <typename T>
    class SymmetricTransformation : public IntervalTransformation<T>
    {
    public:

        /**
         * Create the SymmetricTransformation.
         *
         * @param source_x The source interval [x_min, x_max]
         * @param source_y The source interval [y_min, y_max]
         */
        SymmetricTransformation(Interval<T> source_x, Interval<T> source_y)
        : IntervalTransformation<T>(source_x, source_y, Interval<T>(-1, 1), Interval<T>(-1, 1)) {}

        using IntervalTransformation<T>::project;

    };

    /* Projections */

    /**
     * A projection that maps values using the identity function.
     * [x, y] -> [x, y]
     */
    template <typename T>
    class IdentityProjection : public Transformation<T>
    {
    public:

        using Transformation<T>::Transformation;

        /**
         * Project a pair of values to themselves.
         *
         * @param x The x value
         * @param y The y value
         */
        void transform(T& x, T& y) const override {}

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
    class MercatorProjection : public Transformation<T>
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
        void transform(T& x, T& y) const override
        {
            x = clamp(x - m_center, -M_PI, M_PI);
            y = std::log(std::tan(QUARTER_PI + y / 2));
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
    class CylindricalEqualAreaProjection : public Transformation<T>
    {

        T m_center;
        T m_parallel;

    public:

        /**
         * Create the CylindricalEqualAreaProjection.
         *
         * @param center The central meridian (in longitudes). The default
        *                value 0 specifies the meridian with latitude 0, which
        *                crosses the city of Greenwhich, United Kingdom.
         * @param center The standard parallel (in latitudes). The default value
         *               0 specifies the equator.
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
        void transform(T& x, T& y) const override
        {
            double cos_p = std::cos(m_parallel) + 1e-8;
            x = clamp(x - m_center, -M_PI, M_PI) * cos_p;
            y = std::sin(y) / cos_p;
        }

    };

}
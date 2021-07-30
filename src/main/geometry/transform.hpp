#ifndef MAPMAKER_TRANSFORM_HPP
#define MAPMAKER_TRANSFORM_HPP

#include <cmath>
#include <utility>

#include "geometry/model.hpp"
#include "geometry/util.hpp"

namespace geometry
{

    namespace transform
    {

        /* Helper functions */
        template <typename T>
        T normalize(T value, T min, T max)
        {
            if (value < min) value += std::abs(max - min);
            else if (value > max) value -= std::abs(max - min);
            return value;
        }

        /* Interval */

        template <typename T = double_t>
        class Interval
        {
        public:

            typedef model::Point<T> point_t;

            Interval() {};
            Interval(T left_x, T left_y, T right_x, T right_y)
            : left(left_x, left_y), right(right_x, right_y)
            {
                assert(left_x < right_x && left_y < right_y);
                this->diff_x = right_x - left_x;
                this->diff_y = right_y - left_y;
            };
            Interval(point_t& left, point_t right)
            {
                assert(left.x < right.x && left.y < right.y);
                this->left = left;
                this->right = right;
                this->diff_x = right.x - left.x;
                this->diff_y = right.y - left.y;
            } 

            ~Interval() {};

            /* Members */

            point_t left, right;

            T diff_x, diff_y;

        };

        /* Projection */

        /**
         * Projection class that provides translation functions for two-dimensional points.
         */
        template <typename T = double_t>
        class Projection
        {
        public:

            typedef model::Point<T> point_t;

            virtual point_t project(const point_t& point) const = 0;

        };

        template <typename T = double_t>
        class IdentityProjection : virtual public Projection<T>
        {
        public:

            typedef model::Point<T> point_t;

            point_t project(const point_t& point) const
            {
                return point;
            }

        };

        template <typename T = double_t>
        class GenericProjection : virtual public Projection<T>
        {
        public:

            typedef model::Point<T> point_t;

            explicit GenericProjection(const Interval<T>& source, const Interval<T>& target)
            {
                this->source = source;
                this->target = target;
            }; 

            ~GenericProjection() {};

            point_t project(const point_t& point) const
            {
                T tx = target.left.x + (target.diff_x / source.diff_x) * (point.x - source.left.x);
                T ty = target.left.y + (target.diff_y / source.diff_y) * (point.y - source.left.y);
                return point_t{ tx, ty };
            }

            Interval<T> source;
            Interval<T> target;
        };

        template <typename T = double_t>
        class UnitProjection : public GenericProjection<T>
        {
        public:

            UnitProjection(const Interval<T>& source)
            : GenericProjection<T>(source, Interval<T>{ 0.0, 0.0, 1.0, 1.0 }) {};

            using GenericProjection<T>::project;

        };

        template <typename T = double_t>
        class SymmetricUnitProjection : public GenericProjection<T>
        {
        public:

            SymmetricUnitProjection(const Interval<T>& source)
            : GenericProjection<T>(source, Interval<T>{ -1.0, -1.0, 1.0, 1.0 }) {};

            using GenericProjection<T>::project;

        };

        template <typename T = double_t>
        class MercatorProjection : virtual public Projection<T>
        {
        public:

            typedef model::Point<T> point_t;

            MercatorProjection(T central_meridian = T(0))
            {
                this->central_meridian = central_meridian;
            }

            /* Methods */

            point_t project(const point_t& point) const
            {   
                // Project x and y values
                T tx = normalize(point.x - this->central_meridian, -M_PI, M_PI); // Shift x by the central meridian if specified
                T ty = std::log(std::tan(util::QUARTER_PI + point.y / 2));
                return point_t{ tx, ty };
            }

            /* Members */

            T central_meridian;

        };

        template <typename T = double_t>
        class CylindricalEqualAreaProjection : virtual public Projection<T>
        {
        public:

            typedef model::Point<T> point_t;

            CylindricalEqualAreaProjection(T central_meridian = T(0), T standard_parallel = T(0))
            {
                this->central_meridian = central_meridian;
                this->standard_parallel = standard_parallel;
            }

            /* Methods */

            point_t project(const point_t& point) const
            {   
                // Project x and y values
                double_t cos_p = std::cos(this->standard_parallel);
                T tx = normalize(point.x - this->central_meridian, -M_PI, M_PI) * cos_p; // Shift x by the central meridian if specified
                T ty = std::sin(point.y) / cos_p;
                return point_t{ tx, ty };
            }

            /* Members */

            T central_meridian, standard_parallel;

        };

    }

}

#endif
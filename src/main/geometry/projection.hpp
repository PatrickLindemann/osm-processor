#ifndef MAPMAKER_PROJECTION_HPP
#define MAPMAKER_PROJECTION_HPP

#include <cmath>
#include <utility>

#include "geometry/model.hpp"

namespace geometry
{

    namespace projection
    {

        /* Interval */

        template <typename T = double_t>
        class Interval
        {
        private:

            void set_diffs(T min_x, T min_y, T max_x, T max_y)
            {
                this->diff_x = max_x - min_x;
                this->diff_y = max_y - min_y;
            }

        public:

            Interval() {};
            Interval(T left_x, T left_y, T right_x, T right_y)
            : left(left_x, left_y), right(right_x, right_y)
            {
                assert(left_x < right_x && left_y < right_y);
                set_diffs(left_x, left_y, right_x, right_y);
            };
            Interval(std::pair<T, T>& left, std::pair<T, T> right)
            {
                assert(left.first < right.first && left.second < right.second);
                this->left = left;
                this->right = right;
                set_diffs(left.first, left.second, right.first, right.second);
            }
            Interval(model::Point<T>& left, model::Point<T>& right)
            {
                assert(left.x < right.x && left.y < right.y);
                this->left = std::make_pair(left.x, left.y);
                this->right = std::make_pair(right.x, right.y);
                set_diffs(left.x, left.y, right.x, right.y);
            }

            ~Interval() {};

            /* Members */

            std::pair<T, T> left, right;

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

            explicit Projection(const Interval<T>& interval)
            {
                this->interval = interval;
            }; 

            ~Projection() {};

            /* Methods */

            // virtual std::pair<T, T> translate(T x, T y) const;

            /* Members */

            Interval<T> interval;

        };

        template <typename T = double_t>
        class IdentityProjection : public Projection<T>
        {
        public:

            IdentityProjection(const Interval<T>& interval) : Projection<T>(interval) {};

            const std::pair<T, T> translate(T x, T y) const
            {
                return std::make_pair(x, y);
            }

        };

        template <typename T = double_t>
        class UnitIntervalProjection : public Projection<T>
        {
        public:

            UnitIntervalProjection(const Interval<T>& interval) : Projection<T>(interval) {};

            const std::pair<T, T> translate(T x, T y) const
            {
                T tx = (x - this->interval.left.first) / this->interval.diff_x;
                T ty = (y - this->interval.left.second) / this->interval.diff_y;
                return std::make_pair(T(tx), T(ty));
            }

        };

        template <typename T = double_t>
        class NegativeUnitIntervalProjection : public Projection<T>
        {
        public:

            NegativeUnitIntervalProjection(const Interval<T>& interval) : Projection<T>(interval) {};

            const std::pair<T, T> translate(T x, T y) const
            {
                T tx = (x - this->interval.left.first) / this->interval.diff_x - 1;
                T ty = (y - this->interval.left.second) / this->interval.diff_y - 1;
                return std::make_pair(tx, ty);
            }

        };

        template <typename T = double_t>
        class SymmetricProjection : public Projection<T>
        {
        public:

            SymmetricProjection(const Interval<T>& interval) : Projection<T>(interval) {};

            const std::pair<T, T> translate(T x, T y) const
            {
                T tx = 2 * ((x - this->interval.left.first) / this->interval.diff_x) - 1;
                T ty = 2 * ((y - this->interval.left.second) / this->interval.diff_y) - 1;
                return std::make_pair(tx, ty);
            }

        };

    }

}

#endif
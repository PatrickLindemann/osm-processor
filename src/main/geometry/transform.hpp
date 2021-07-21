#ifndef MAPMAKER_TRANSFORM_HPP
#define MAPMAKER_TRANSFORM_HPP

#include <cmath>
#include <utility>

namespace geometry
{

    namespace transform
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

            explicit Projection(const Interval<T>& source, const Interval<T>& target)
            {
                this->source = source;
                this->target = target;
            }; 

            ~Projection() {};

            /* Methods */

            std::pair<T, T> project(T x, T y) const
            {
                T tx = target.left.first + (target.diff_x / source.diff_x) * (x - source.left.first);
                T ty = target.left.second + (target.diff_y / source.diff_y) * (y - source.left.second);
                return std::make_pair(tx, ty);
            }

            std::pair<T, T> project(std::pair<T, T> xy) const
            {
                return this->project(xy.first, xy.second);
            }

            /* Members */

            Interval<T> source;
            Interval<T> target;

        };

        template <typename T = double_t>
        class IdentityProjection : public Projection<T>
        {
        public:

            IdentityProjection(const Interval<T>& source)
            {
                this->source = source;
                this->target = source;
            }

            std::pair<T, T> project(T x, T y) const
            {
                return std::make_pair(x, y);
            }

            std::pair<T, T> project(std::pair<T, T> xy) const
            {
                return xy;
            }

        };

        template <typename T = double_t>
        class UnitProjection : public Projection<T>
        {
        public:

            UnitProjection(const Interval<T>& source)
            : Projection<T>(source, Interval<T>{ 0.0, 0.0, 1.0, 1.0 }) {};

            using Projection<T>::project;

        };

        template <typename T = double_t>
        class SymmetricUnitProjection : public Projection<T>
        {
        public:

            SymmetricUnitProjection(const Interval<T>& source)
            : Projection<T>(source, Interval<T>{ -1.0, -1.0, 1.0, 1.0 }) {};

            using Projection<T>::project;

        };

        /**
         * https://en.wikipedia.org/wiki/Web_Mercator_projection
         */
        template <typename T = double_t>
        class WebMercatorProjection : public Projection<T>
        {
        public:

            WebMercatorProjection(const Interval<T>& source)
            : Projection<T>(source, Interval<T>{ 0.0, 0.0, 1.0, 1.0 }) {};

            std::pair<T, T> project(T x, T y) const
            {
                T tx = std::floor(1.0 / (2 * M_PI) * std::exp2(6) * (x + M_PI));
                T ty = std::floor(1.0 / (2 * M_PI) * std::exp2(6) * (M_PI - std::log(std::tan(M_PI / 4 + y / 2))));
                return std::make_pair(x, y);
            }

            std::pair<T, T> project(std::pair<T, T> xy) const
            {
                return project(xy.first, xy.second);
            }   

        };

    }

}

#endif
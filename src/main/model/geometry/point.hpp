#pragma once

namespace model
{

    namespace geometry
    {

        /**
         * 
         */
        template <typename T>
        class Point
        {
        public:

            /* Types */

            using value_type = T;

            /* Members */
            
            T x, y;

            /* Constructor */

            Point(T x, T y) : x(x), y(y) {};

            /* Operators */

            Point<T> operator+(const Point<T>& other) const
            {
                return Point<T>{ x + other.x, y + other.y };
            }

            Point<T> operator-(const Point<T>& other) const
            {
                return Point<T>{ x - other.x, y - other.y };
            }

            Point<T> operator*(const T value) const
            {
                return Point<T>{ x * value, y * value };
            }

            Point<T> operator/(const T value) const
            {
                return Point<T>{ x / value, y / value };
            }

            Point<T>& operator+=(const Point<T>& other)
            {
                x += other.x;
                y += other.y;
                return *this;
            }

            Point<T>& operator-=(const Point<T>& other)
            {
                x -= other.x;
                y -= other.y;
                return *this;
            }

            Point<T>& operator*=(const T value)
            {
                x *= value;
                y *= value;
                return *this;
            }

            Point<T>& operator/=(const T value)
            {
                x /= value;
                y /= value;
                return *this;
            }

            bool operator==(const Point<T>& other) const
            {
                return x == other.x && y == other.y;
            }

            bool operator!=(const Point<T>& other) const
            {
                return x != other.x || y != other.y;
            }

        };

    }

}
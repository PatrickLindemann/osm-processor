#pragma once

namespace model
{

    namespace geometry
    {

        template <typename T>
        class Point
        {
            /* Members */
            
            T m_x, m_y;

        public:

            /* Constructors */

            Point() : m_x(0), m_y(0) {};
            Point(T x, T y) : m_x(x), m_y(y) {};

            /* Accessors */

            T& x()
            {
                return m_x;
            }

            const T& x() const
            {
                return m_x;
            }

            T& y()
            {
                return m_y;
            }

            const T& y() const
            {
                return m_y;
            }

            /* Operators */

            Point<T> operator+(const Point<T>& other) const
            {
                return Point<T>{ m_x + other.x(), m_y + other.y() };
            }

            Point<T> operator-(const Point<T>& other) const
            {
                return Point<T>{ m_x - other.x(), m_y - other.y() };
            }

            Point<T> operator*(const T value) const
            {
                return Point<T>{ m_x * value, m_y * value };
            }

            Point<T> operator/(const T value) const
            {
                return Point<T>{ m_x / value, m_y / value };
            }

            Point<T>& operator+=(const Point<T>& other)
            {
                m_x += other.x();
                m_y += other.y();
                return *this;
            }

            Point<T>& operator-=(const Point<T>& other)
            {
                m_x -= other.x();
                m_y -= other.y();
                return *this;
            }

            Point<T>& operator*=(const T value)
            {
                m_x *= value;
                m_y *= value;
                return *this;
            }

            Point<T>& operator/=(const T value)
            {
                m_x /= value;
                m_y /= value;
                return *this;
            }

            bool operator==(const Point<T>& other) const
            {
                return m_x == other.x() && m_y == other.y();
            }

            bool operator!=(const Point<T>& other) const
            {
                return m_x != other.x() || m_y != other.y();
            }

        };

    }

}
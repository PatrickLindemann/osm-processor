#ifndef GEOMETRY_MODEL_HPP
#define GEOMETRY_MODEL_HPP

#include <cmath>
#include <math.h>
#include <array>
#include <vector>
#include <initializer_list>

namespace geometry
{

    namespace model
    {

        /* Base class */

        class Entity {};

        enum Type
        {
            ePoint,
            eSegment,
            eLine,
            eRectangle,
            eCircle,
            ePolygon,
            eMultiPolygon
        };

        /* Point */

        template <typename T = double_t>
        class Point : public Entity
        {
        public:

            Point() : x(T(0)), y(T(0)) {};
            Point(T x, T y) { this->x = x; this->y = y; };
            Point(const Point& p) : x(p.x), y(p.y) {};

            ~Point() {};

            /* Arithmetic operators */

            Point operator+(const Point& other) const
            {
                return Point{this->x + other.x, this->y + other.y};
            }

            Point operator-(const Point& other) const
            {
                return Point{this->x - other.x, this->y - other.y};
            }

            Point operator*(const T value) const
            {
                return Point{this->x * value, this->y * value};
            }

            Point operator/(const T value) const
            {
                return Point{this->x / value, this->y / value};
            }

            /* Assignment operators */

            Point& operator+=(const Point& other)
            {
                this->x += other.x;
                this->y += other.y;
                return *this;
            }

            Point& operator-=(const Point& other)
            {
                this->x -= other.x;
                this->y -= other.y;
                return *this;
            }

            Point& operator*=(const T value)
            {
                this->x *= value;
                this->y *= value;
                return *this;
            }

            Point& operator/=(const T value)
            {
                this->x /= value;
                this->y /= value;
                return *this;
            }

            /* Comparison operators */

            bool operator==(const Point& other) const
            {
                return this->x == other.x && this->y == other.y;
            }

            bool operator!=(const Point& other) const
            {
                return this->x != other.x || this->y != other.y;
            }

            /* Members */

            T x, y;
            
        };

        /* Segment */

        template <typename T = double_t>
        class Segment : public Entity
        {
        public:

            typedef Point<T> point_type;
            typedef std::array<point_type, 2> point_array_type;
            typedef typename point_array_type::iterator iterator;
            typedef typename point_array_type::const_iterator const_iterator;

            Segment() {};
            Segment(point_type& first, point_type& last) : points(first, last) {};

            ~Segment() {};

            /* Accessors */

            const point_type& first() const
            {
                return points[0];
            }

            const point_type& last() const
            {
                return points[1];
            }

            /* Accessor operators */

            point_type& operator[](const std::size_t index)
            {
                return points[index];
            }

            const point_type& operator[](const std::size_t index) const
            {
                return points[index];
            }

            /* Vector methods */

            const bool valid() const
            {
                return points.size() == 2;
            } 

            const bool empty() const
            {
                return points.empty();
            }

            std::size_t size() const
            {
                return points.size();
            }

            iterator begin() noexcept
            {
                return points.begin();
            }

            const_iterator cbegin() const noexcept
            {
                return points.cbegin();
            }

            iterator end() noexcept
            {
                return points.end();
            }

            const_iterator cend() const noexcept
            {
                return points.cend();
            }

            /* Members */

            point_array_type points;

        };

        /* Line */

        template <typename T = double_t>
        class Line : public Entity
        {
        public:

            typedef Point<T> point_type;
            typedef std::vector<point_type> point_array_type;
            typedef typename point_array_type::iterator iterator;
            typedef typename point_array_type::const_iterator const_iterator;

            Line() {};
            Line(point_array_type& points) { this->points = points; };
            Line(std::initializer_list<point_type>& points) { this->points = points; };

            ~Line() {};

            /* Accessors */

            const point_type& first() const
            {
                return points[0];
            }

            const point_type& last() const
            {
                return points[points.size() - 1];
            }

            /* Accessor operators */

            point_type& operator[](const std::size_t index)
            {
                return points[index];
            }

            const point_type& operator[](const std::size_t index) const
            {
                return points[index];
            }

            /* Vector methods */

            const bool valid() const
            {
                return points.size() >= 2;
            } 

            const bool empty() const
            {
                return points.empty();
            }

            std::size_t size() const
            {
                return points.size();
            }

            iterator begin() noexcept
            {
                return points.begin();
            }

            const_iterator cbegin() const noexcept
            {
                return points.cbegin();
            }

            iterator end() noexcept
            {
                return points.end();
            }

            const_iterator cend() const noexcept
            {
                return points.cend();
            }

            /* Members */

            point_array_type points;

        };

        /* Rectangle */

        template <typename T = double_t>
        class Rectangle : public Entity
        {
        public:

            typedef Point<T> point_type;

            Rectangle() {};
            Rectangle(T min_x, T min_y, T max_x, T max_y) : min(min_x, min_y), max(max_x, max_y) {};
            Rectangle(point_type& min, point_type& max) { this->min = min; this->max = max; };

            ~Rectangle() {};

            /* Accessors */

            const point_type bottom_left() const
            {
                return point_type{ this->min };
            }

            const point_type top_left() const
            {
                return point_type{ this->min.x, this->max.y };
            }

            const point_type top_right() const
            {
                return point_type{ this->max };
            }

            const point_type bottom_right() const
            {
                return point_type{ this->max.x, this->min.y };
            }

            /* Misc */

            const bool valid() const
            {
                return this->min.x <= this->max.x && this->min.y <= this->max.y; 
            }

            const T width() const
            {
                return this->max.x -  this->min.x;
            }

            const T height() const
            {
                return this->max.y - this->min.y;
            }

            const double area() const
            {
                return this->width() * this->height();
            }

            const double circumference() const
            {
                return 2 * (this->width() + this->height());
            }

            /* Members */

            point_type min, max;

        };

        /* Circle */

        template <typename T = double_t>
        class Circle : public Entity
        {
        public:

            typedef Point<T> point_type;

            Circle() {};
            Circle(point_type& center) { this->center = center; };
            Circle(point_type& center, T radius) { this->center = center; this->radius = radius; };

            ~Circle() {};

            /* Misc */

            const bool valid() const
            {
                return radius >= 0;
            }

            const T diameter() const
            {
                return this->radius * 2;
            }

            const double area() const
            {
                return M_PI * std::pow(this->radius, 2);
            }

            const double circumference() const
            {
                return 2 * this->radius * M_PI;
            }

            /* Members */

            point_type center;

            T radius;

        };

        /* Polygon */

        template <typename T = double_t>
        class Polygon : public Entity
        {
        public:

            typedef Point<T> point_type;
            typedef std::vector<point_type> point_array_type;
            typedef point_array_type outer_type;
            typedef std::vector<point_array_type> inner_type;

            Polygon() {};
            Polygon(outer_type& outer) { this->outer = outer; };
            Polygon(outer_type& outer, inner_type& inners) { this->outer = outer; this->inners = inners; };

            ~Polygon() {};

            /* Misc */

            const bool valid() const
            {
                return true;
            }

            /* Members */

            outer_type outer;

            inner_type inners;

        };

        /* MultiPolygon */
        
        template <typename T = double_t>
        class MultiPolygon : public Entity
        {
        public:

            typedef Polygon<T> polygon_type;
            typedef std::vector<polygon_type> polygon_array_type;

            MultiPolygon() {};
            MultiPolygon(polygon_array_type& polygons) { this->polygons = polygons; };
            MultiPolygon(std::initializer_list<polygon_type>& polygons) { this->poylgons = polygons; };

            ~MultiPolygon() {};

            /* Misc */

            const bool valid() const
            {
                for (const polygon_type& p : polygons)
                    if (!p.valid())
                        return false;
                return true;
            }

            /* Members */

            polygon_array_type polygons;

        };
    
    }

}

#endif
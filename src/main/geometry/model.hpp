#ifndef GEOMETRY_MODEL_HPP
#define GEOMETRY_MODEL_HPP

#include <cmath>
#include <limits>
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

            typedef Point<T> point_t;
            typedef std::array<point_t, 2> point_list;
            typedef typename point_list::iterator iterator;
            typedef typename point_list::const_iterator const_iterator;

            Segment() {};
            Segment(point_t& first, point_t& last) : points(first, last) {};

            ~Segment() {};

            /* Accessors */

            const point_t& first() const
            {
                return points[0];
            }

            const point_t& last() const
            {
                return points[1];
            }

            /* Accessor operators */

            point_t& operator[](const std::size_t index)
            {
                return points[index];
            }

            const point_t& operator[](const std::size_t index) const
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

            point_list points;

        };

        /* Line */

        template <typename T = double_t>
        class Line : public Entity
        {
        public:

            typedef Point<T> point_t;
            typedef std::vector<point_t> point_list;
            typedef typename point_list::iterator iterator;
            typedef typename point_list::const_iterator const_iterator;

            Line() {};
            Line(point_list& points) { this->points = points; };
            Line(std::initializer_list<point_t>& points) { this->points = points; };

            ~Line() {};

            /* Accessors */

            const point_t& first() const
            {
                return points[0];
            }

            const point_t& last() const
            {
                return points[points.size() - 1];
            }

            /* Accessor operators */

            point_t& operator[](const std::size_t index)
            {
                return points[index];
            }

            const point_t& operator[](const std::size_t index) const
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

            point_list points;

        };

        /* Rectangle */

        template <typename T = double_t>
        class Rectangle : public Entity
        {
        public:

            typedef Point<T> point_t;

            Rectangle() {};
            Rectangle(T min_x, T min_y, T max_x, T max_y) : min(min_x, min_y), max(max_x, max_y) {};
            Rectangle(point_t& min, point_t& max) { this->min = min; this->max = max; };

            ~Rectangle() {};

            /* Accessors */

            const point_t bottom_left() const
            {
                return point_t{ this->min };
            }

            const point_t top_left() const
            {
                return point_t{ this->min.x, this->max.y };
            }

            const point_t top_right() const
            {
                return point_t{ this->max };
            }

            const point_t bottom_right() const
            {
                return point_t{ this->max.x, this->min.y };
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

            Rectangle<T>& extend(const point_t& p) {
                if (p.x < this->min.x) this->min.x = p.x;
                if (p.y < this->min.y) this->min.y = p.y;
                if (p.x > this->max.x) this->max.x = p.x;
                if (p.y < this->max.y) this->max.y = p.y;
                return *this;
            }

            Rectangle<T>& extend(const Rectangle<T>& other) {
                if (other.min.x < this->min.x) this->min.x = other.min.x;
                if (other.min.y < this->min.y) this->min.y = other.min.y;
                if (other.max.x > this->max.x) this->max.x = other.max.x;
                if (other.max.y > this->max.y) this->max.y = other.max.y;
                return *this;
            }

            /* Members */

            point_t min, max;

        };

        /* Circle */

        template <typename T = double_t>
        class Circle : public Entity
        {
        public:

            typedef Point<T> point_t;

            Circle() {};
            Circle(point_t& center) { this->center = center; };
            Circle(point_t& center, T radius) { this->center = center; this->radius = radius; };

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

            point_t center;

            T radius;

        };

        /* Polygon */

        template <typename T = double_t>
        class Polygon : public Entity
        {
        public:

            typedef Point<T> point_t;
            typedef std::vector<point_t> point_list;

            Polygon() {};
            Polygon(point_list& outer) { this->outer = outer; };
            Polygon(point_list& outer, std::vector<point_list>& inners) { this->outer = outer; this->inners = inners; };

            ~Polygon() {};

            /* Misc */

            const bool valid() const
            {
                return true;
            }

            /* Members */

            point_list outer;
            std::vector<point_list> inners;

        };

        /* MultiPolygon */
        
        template <typename T = double_t>
        class MultiPolygon : public Entity
        {
        public:

            typedef Polygon<T> polygon_t;
            typedef std::vector<polygon_t> polygon_list;

            MultiPolygon() {};
            MultiPolygon(polygon_list& polygons) { this->polygons = polygons; };
            MultiPolygon(std::initializer_list<polygon_t>& polygons) { this->poylgons = polygons; };

            ~MultiPolygon() {};

            /* Misc */

            const bool valid() const
            {
                for (const polygon_t& p : polygons)
                    if (!p.valid())
                        return false;
                return true;
            }

            const bool is_polygon() const
            {
                return this->polygons.size() == 1;
            }

            /* Members */

            polygon_list polygons;

        };
    
    }

}

#endif
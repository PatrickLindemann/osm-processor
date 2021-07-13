#ifndef MAPMAKER_POINT_HPP
#define MAPMAKER_POINT_HPP

#include <cmath>
#include <vector>
#include <ostream>

#include "model/entity.hpp"

namespace mapmaker
{

class Point : public Entity
{

public:

    double_t x, y;

    // constructors
    Point() : x(0.0), y(0.0) {}
    Point(double_t xx, double_t yy) : x(xx), y(yy) {}

    // destructor
    ~Point() {}

    const Point& operator+=(const Point& rhs)
    {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    const Point& operator-=(const Point& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }

    const Point& operator/=(const double_t rhs)
    {
        x /= rhs;
        y /= rhs;
        return *this;
    }

    const Point& operator*=(const double_t rhs)
    {
        x *= rhs;
        y *= rhs;
        return *this;
    }

    inline friend Point operator+(Point lhs, const Point& rhs)
    {
        return Point{ lhs.x + rhs.x, lhs.y + rhs.y};
    }

    inline friend Point operator-(Point lhs, const Point& rhs)
    {
        return Point{ lhs.x - rhs.x, lhs.y - rhs.y};
    }

    inline friend Point operator*(Point lhs, const double_t rhs)
    {
        return Point{ lhs.x * rhs, lhs.y * rhs};
    }

    inline friend Point operator/(Point lhs, const double_t rhs)
    {
        return Point{ lhs.x / rhs, lhs.y / rhs};
    }

    inline friend bool operator==(const Point& lhs, const Point& rhs)
    {
        return lhs.x == rhs.x && lhs.y == rhs.y;
    }

    inline friend bool operator!=(const Point& lhs, const Point& rhs)
    {
        return !(lhs == rhs);
    }

    inline friend std::ostream& operator<<(std::ostream& outs, const Point& p)
    {
        return outs << "Point(" << p.x << "," << p.y << ")";
    }

};

typedef std::vector<Point> PointList;

}

#endif
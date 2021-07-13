#ifndef MAPMAKER_BOX_HPP
#define MAPMAKER_BOX_HPP

#include <cassert>
#include <cmath>

#include "model/entity.hpp"
#include "model/point.hpp"

namespace mapmaker
{

    class Box : public Entity {

    public:
    
        Point min_corner, max_corner;

        Box() : min_corner(), max_corner() {};

        Box(double min_x, double min_y, double max_x, double max_y) :
            min_corner(min_x, min_y),
            max_corner(max_x, max_y)
        {
            assert(min_x <= max_x && min_y <= max_y);
        };
        
        Box(const Point min, const Point max) :
            min_corner(min),
            max_corner(max)
        {
            assert(min.x <= max.x && min.y <= max.y);
        };

        Box& extend(const Point& p)
        {
            if (p.x < min_corner.x)
                min_corner.x = p.x;
            if (p.y < min_corner.y)
                min_corner.y = p.y;
            if (p.x > max_corner.x)
                max_corner.x = p.x;
            if (p.y > max_corner.y)
                max_corner.y = p.y;
            return *this;
        }

        Box& extend(const Box& box)
        {
            extend(box.min_corner);
            extend(box.min_corner);
            return *this;
        }

        Point bottom_left() const
        {
            return Point{ min_corner.x, min_corner.y };
        }

        Point bottom_right() const
        {
            return Point{ max_corner.x, min_corner.y };
        }

        Point top_left() const
        {
            return Point{ min_corner.x, max_corner.y };
        }

        Point top_right() const
        {
            return Point{ max_corner.x, max_corner.y };
        }

        double_t width() const
        {
            return max_corner.x - min_corner.x;
        }

        double_t height() const
        {
            return max_corner.y - min_corner.y;
        }

    };

}

#endif
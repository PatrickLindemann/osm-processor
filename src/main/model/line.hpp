#ifndef MAPMAKER_LINE_HPP
#define MAPMAKER_LINE_HPP

#include <deque>
#include <vector>

#include "model/box.hpp"
#include "model/point.hpp"
#include "model/entity.hpp"

#include "functions.hpp"

namespace mapmaker
{

class Line : public Entity
{

    PointList m_points;

public:
    
    Line(PointList& points) : m_points(points) {
        assert(points.size() >= 2);
    };

    Line(std::initializer_list<Point>& points) : m_points(points) {
        assert(points.size() >= 2);
    };

    ~Line() {};

    const PointList& points()
    {
        return m_points;
    }

    const PointList points() const
    {
        return m_points;
    }

    const size_t size() const
    {
        return m_points.size();
    }

    const Box bounds()
    {
        return functions::calc_bounds(m_points);
    }

    const Point& operator[](const size_t index)
    {
        return m_points[index];
    }

    auto begin() noexcept
    {
        return m_points.begin();
    }

    auto end() noexcept
    {
        return m_points.end();
    }

};

}

#endif
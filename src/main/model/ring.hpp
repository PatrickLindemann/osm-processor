#ifndef MAPMAKER_RING_HPP
#define MAPMAKER_RING_HPP

#include <cassert>
#include <initializer_list>

#include "model/entity.hpp"
#include "model/point.hpp"

#include "functions.hpp"

namespace mapmaker
{

class Ring : public Entity
{

    PointList m_points;

    private:

        void close(PointList& points)
        {
            size_t end = points.size() - 1;
            // Ensure that the first and last point of the ring are the same,
            // so that the ring is closed
            if (points[0] != points[end])
                points.push_back(points[0]);
        }

    public:

        Ring() {};

        Ring(const PointList& points) : m_points(points)
        {
            // A valid ring has to have at least three unique nodes
            assert(m_points.size() >= 3);
            close(m_points);
            // Check if the ring is now closed
            assert(m_points.size() >= 4);
        };

        Ring(std::initializer_list<Point> points) : m_points(points)
        {
            // A valid ring has to have at least three unique nodes
            assert(m_points.size() >= 3);
            close(m_points);
            // Check if the ring is now closed
            assert(m_points.size() >= 4);
        }

        const PointList& points() const
        {
            return m_points;
        }

        const bool empty() const
        {
            return m_points.empty();
        }

        const size_t size() const
        {
            return m_points.size();
        }

        auto begin() noexcept
        {
            return m_points.begin();
        }

        auto end() noexcept
        {
            return m_points.end();
        }

        const Point& operator[](const size_t index) const
        {
            return m_points[index];
        }

};

}

#endif
#pragma once

#include <limits>

#include "model/memory/node.hpp"
#include "model/memory/buffer.hpp"
#include "model/geometry/point.hpp"
#include "model/geometry/rectangle.hpp"
#include "functions/project.hpp"

namespace mapmaker
{

    namespace projector
    {

        using namespace model;

        template <typename T>
        class Projector
        {

            memory::Buffer<memory::Node<T>>& m_nodes;

        public:

            Projector(
                memory::Buffer<memory::Node<T>>& nodes
            ) : m_nodes(nodes) {};

            /**
             * Apply a projection to the internal node objects.
             */
            void apply(const functions::Projection<T>& projection)
            {
                for (auto& node : m_nodes)
                {
                    node.point() = projection.project(node.lon(), node.lat());
                }
            }

            geometry::Rectangle<T> bounds()
            {
                std::numeric_limits<T> limits;
                T lon_min = limits.max();
                T lon_max = -limits.max();
                T lat_min = limits.max();
                T lat_max = -limits.max();
                for (const memory::Node<T>& node : m_nodes)
                {
                    lon_min = std::min(lon_min, node.lon());
                    lon_max = std::max(lon_max, node.lon());
                    lat_min = std::min(lat_min, node.lat());
                    lat_max = std::max(lat_max, node.lat());
                }
                return geometry::Rectangle<T>{ { lon_min, lat_min }, { lon_max, lat_max }};
            }

        };

    }

}
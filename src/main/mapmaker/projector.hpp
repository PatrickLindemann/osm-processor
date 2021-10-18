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

        using namespace model::memory;
        using namespace model::geometry;

        template <typename T>
        class Projector
        {

            Buffer<Node>& m_nodes;

        public:

            Projector(Buffer<Node>& nodes) : m_nodes(nodes) {};

            /**
             * Apply a projection to the internal node objects.
             */
            void apply_projection(const functions::Projection<T>& projection)
            {
                for (auto& node : m_nodes)
                {
                    node.point() = projection.project(node.lon(), node.lat());
                }
            }

        };

    }

}
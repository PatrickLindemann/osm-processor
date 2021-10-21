#pragma once

#include <limits>

#include "functions/project.hpp"
#include "functions/transform.hpp"
#include "model/geometry/point.hpp"
#include "model/geometry/rectangle.hpp"
#include "model/memory/area.hpp"
#include "model/memory/buffer.hpp"
#include "model/memory/node.hpp"

namespace mapmaker
{

    namespace transformer
    {

        template <typename T>
        class Transformer
        {

            memory::Buffer<memory::Node>& m_node_buffer;

        public:

            Transformer(memory::Buffer<memory::Node>& nodes) : m_node_buffer(nodes) {};

            /**
             * Apply a projection to the internal node objects.
             */
            void apply(const functions::Projection<T>& projection)
            {
                for (memory::Node& node : m_node_buffer)
                {
                    node.point() = projection.project(node.lon(), node.lat());
                }
            }

            /**
             * Apply a transformation to the internal node objects.
             */
            void apply(const functions::Transformation<T>& transformation)
            {
                for (memory::Node& node : m_node_buffer)
                {
                    node.point() = transformation.transform(node.lon(), node.lat());
                }
            }

            /**
             * Retrieve the boundign box of the projected nodes for a
             * specified set of areas.
             * 
             * @param areas The area buffer
             * @returns     The bounding box of the areas in respect to
             *              to projected nodes
             * 
             * Time complexity: Linear
             */
            geometry::Rectangle<T> get_bounds(const memory::Buffer<memory::Area>& areas)
            {
                std::numeric_limits<T> limits;
                T x_min = limits.max();
                T y_min = limits.max();
                T x_max = -limits.max();
                T y_max = -limits.max();
                for (const memory::Area& area : areas)
                {
                    for (const memory::Ring& outer : area.outer_rings())
                    {
                        for (const memory::NodeRef& nr : outer)
                        {
                            const memory::Node& node = m_node_buffer.at(nr);
                            x_min = std::min(x_min, node.point().x());
                            y_min = std::min(y_min, node.point().y());
                            x_max = std::max(x_max, node.point().x());
                            y_max = std::max(y_max, node.point().y());
                        }
                    }
                }
                return geometry::Rectangle<double>{ x_min, y_min, x_max, y_max };
            }

        };

    }

}
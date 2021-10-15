#pragma once

#include "model/geometry/point.hpp"
#include "model/geometry/rectangle.hpp"
#include "model/geometry/polygon.hpp"
#include "model/geometry/multipolygon.hpp"
#include "model/memory/area.hpp"
#include "model/memory/buffer.hpp"
#include "model/memory/node.hpp"
#include "model/memory/ring.hpp"
#include "model/memory/types.hpp"
#include "model/memory/way.hpp"

namespace mapmaker
{

    namespace converter
    {

        using namespace model;

        template <typename T>
        class GeometryConverter
        {
        public:

            using ring_type         = geometry::Ring<T>;
            using polygon_type      = geometry::Polygon<T>;
            using multipolygon_type = geometry::MultiPolygon<T>;

            using id_type           = memory::object_id_type;
            using container_type    = std::vector<multipolygon_type>;

        protected:

            const memory::Buffer<memory::Node<T>>& m_nodes;
            const memory::Buffer<memory::Area<T>>& m_areas;

            container_type m_geometries;

        public:

            GeometryConverter(
                const memory::Buffer<memory::Node<T>>& nodes,
                const memory::Buffer<memory::Area<T>>& areas
            ) : m_nodes(nodes), m_areas(areas) {};

            container_type geometries()
            {
                return m_geometries;
            }

        protected:

            /**
             * 
             */
            const ring_type convert_ring(const memory::Ring<T>& ring) const
            {
                ring_type result;
                for (const id_type& node_ref : ring)
                {
                    const memory::Node<T>& node = m_nodes.get(node_ref);
                    result.push_back(node.point());
                }
                return result;
            }

        public:

            /**
             * 
             */
            void run() {
                for (const memory::Area<T>& area : m_areas)
                {
                    multipolygon_type multipolygon;
                    for (const memory::Ring<T>& outer : area.outer_rings())
                    {
                        polygon_type polygon;
                        polygon.outer = convert_ring(outer);
                        for (const memory::Ring<T> inner : area.inner_rings(outer))
                        {
                            polygon.inners.push_back(convert_ring(outer));
                        }
                        multipolygon.polygons.push_back(polygon);
                    }
                    m_geometries.push_back(multipolygon);
                }
            }

        };


    }

}
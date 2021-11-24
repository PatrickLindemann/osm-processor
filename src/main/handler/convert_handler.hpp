#pragma once

#include <cstddef>

#include <osmium/handler.hpp>
#include <osmium/osm/area.hpp>
#include <osmium/osm/node.hpp>
#include <osmium/osm/node_ref.hpp>
#include <osmium/osm/node_ref_list.hpp>
#include <osmium/osm/item_type.hpp>

#include "functions/transform.hpp"
#include "model/types.hpp"

using namespace model;

namespace handler
{

    template <typename T>
    class BoundaryConvertHandler : public osmium::handler::Handler
    {
    protected:

        /* Types */

        using Transformation = std::shared_ptr<functions::Transformation<T>>;;

        /* Members */

        /**
         *
         */
        std::vector<Transformation> m_transformations;

       /**
        *
        */
        std::map<object_id_type, Boundary<T>> m_boundaries;

    public:

        /* Constructors */

        BoundaryConvertHandler() {}
        BoundaryConvertHandler(Transformation transformation) : m_transformations({ transformation }) {}
        BoundaryConvertHandler(const std::vector<Transformation>& transformations) : m_transformations(transformations) {}

        /* Accessors */

        const std::vector<Transformation>& transformations() const
        {
            return m_transformations;
        }

        const std::map<object_id_type, Boundary<T>>& boundaries() const
        {
            return m_boundaries;
        }

    protected:

        /* Helper Methods */

        /**
         * Convert an osmium ring of to a ring geometry by resolving the node
         * references and applying the specified transformations.
         *
         * @param node_refs The area ring, which extends osmium::NodeRefList
         * @returns         The ring geometry
         *
         * Time complexity: Linear
         */
        geometry::Ring<T> create_ring(const osmium::NodeRefList& node_refs)
        {
            geometry::Ring<T> ring;
            for (const osmium::NodeRef& nr : node_refs)
            {
                // Apply the transformations on the node
                T x = nr.lon();
                T y = nr.lat();
                for (const Transformation& transformation : m_transformations)
                {
                    transformation->transform(x, y);
                }
                ring.push_back({ x, y });
            }
            return ring;
        }

    public:

        /* Osmium Methods */

        void area(const osmium::Area& area) noexcept
        {
            // Create the multipolygon geometry for the area
            geometry::MultiPolygon<T> multipolygon;
            // Create a polygon with one outer and N inner rings for each outer
            // ring of the area
            for (const osmium::OuterRing& outer : area.outer_rings())
            {
                geometry::Polygon<T> polygon{ create_ring(outer) };
                // Add the inner rings of the area to the polygon
                for (const osmium::InnerRing& inner : area.inner_rings(outer))
                {
                    polygon.inners().push_back(create_ring(inner));
                }
                // Add the finished polygon to the multipolygon geometry
                multipolygon.polygons().push_back(polygon);
            }
            // Create the boundary with the converted geometry and other area
            // tag values and add it to the boundary map.
            Boundary<T> boundary{
                area.id(),
                area.get_value_by_key("name", ""),
                boost::lexical_cast<level_type>(area.get_value_by_key("admin_level", "0")),
                multipolygon
            };
            m_boundaries[area.id()] = boundary;
        }

    };

}
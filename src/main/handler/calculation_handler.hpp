#pragma once

#include <set>

#include <osmium/handler.hpp>
#include <osmium/osm/area.hpp>
#include <osmium/osm/node_ref_list.hpp>

#include "functions/area.hpp"

namespace handler
{

    /**
     * A handler that calculates the surface area of osmium areas.
     */
    class SurfaceAreaHandler : public osmium::handler::Handler
    {
    protected:

        /* Members */

        std::map<osmium::object_id_type, double> m_surfaces;

        double m_total;

    public:

        /* Constructors */

        SurfaceAreaHandler() {}

        /* Accessors */

        const std::map<osmium::object_id_type, double>& surfaces() const
        {
            return m_surfaces;
        };

        double total() const
        {
            return m_total;
        }

    protected:

        /* Helper Methods */

        /**
         * Calculate the (signed) surface area of a ring. If the nodes are
         * defined in counter-clockwise order, the result will be positive,
         * if they are defined clockwise, the result will be negative.
         *
         * For more information and proof of this formula, refer to
         * https://en.wikipedia.org/wiki/Shoelace_formula.
         *
         * @param node_refs  The ring
         * @returns          The surface area of the ring
         *
         * Time complexity: Linear
         */
        double surface_area(const osmium::NodeRefList& node_refs)
        {
            double left_sum = 0.0;
            double right_sum = 0.0;

            for (std::size_t i = 0; i < node_refs.size() - 1; i++)
            {
                left_sum += node_refs[i].lon() * node_refs[i + 1].lat();
                right_sum += node_refs[i + 1].lon() * node_refs[i].lat();
            }

            return 0.5 * left_sum - right_sum;
        }

    public:

        /* Osmium Methods */

        void area(const osmium::Area& area) noexcept
        {
            double a = 0.0;
            for (const osmium::OuterRing& outer : area.outer_rings())
            {
                // Calculate and add the surface area of the outer ring. Outer
                // rings are defined in counter-clockwise order by the assembler,
                // so the calculated surface area will be positive.
                a += surface_area(outer);
                for (const osmium::InnerRing& inner : area.inner_rings(outer))
                {
                    // Calculate and subtract the surface area of the inner ring.
                    // Inner rings are defined in clockwise order by the
                    // assembler, so the calculated surface area will be negative
                    // and is actually added instead of subtracted.
                    a += surface_area(inner);
                }
            }
            m_surfaces[area.id()] = a;
            m_total += a;
        }

    };

}
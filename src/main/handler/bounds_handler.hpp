#pragma once

#include <osmium/handler.hpp>
#include <osmium/osm/box.hpp>
#include <osmium/osm/node.hpp>

namespace handler
{

    /**
     * A handler that determines the bounding box of an osmium object stream.
     */
    class BoundsHandler : public osmium::handler::Handler
    {
    protected:

        /* Members */

        osmium::Box m_bounds;

    public:

        /* Accessors */

        const osmium::Box& bounds() const
        {
            return m_bounds;
        };

        /* Osmium functions */

        void node(const osmium::Node& node) noexcept
        {
            m_bounds.extend(node.location());
        }

    };

}
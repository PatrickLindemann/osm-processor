#pragma once

#include <cstddef>

#include <osmium/osm/node.hpp>
#include <osmium/osm/way.hpp>
#include <osmium/osm/relation.hpp>
#include <osmium/osm/area.hpp>
#include <osmium/osm/item_type.hpp>
#include <osmium/memory/buffer.hpp>

namespace mapmaker
{

    template <osmium::item_type T>
    class Counter
    {
    public:

        /* Constructors */

        Counter() {}

        /* Methods */

        const std::size_t run(const osmium::memory::Buffer& buffer) const
        {           
            // Create a count handler for the type and apply it to the specified
            // buffer.
            handler::CountHandler count_handler{ T };
            osmium::apply(buffer, count_handler);
            return count_handler.count(T);
        }

    };

    class NodeCounter : public Counter<osmium::item_type::node>{};
    class WayCounter : public Counter<osmium::item_type::way>{};
    class RelationCounter : public Counter<osmium::item_type::relation>{};
    class AreaCounter : public Counter<osmium::item_type::area>{};

}
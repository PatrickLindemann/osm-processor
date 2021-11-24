#pragma once

#include <cstddef>

#include <osmium/handler.hpp>
#include <osmium/osm/item_type.hpp>

namespace handler
{

    /**
     * A handler that counts the total number of occurences of objects in an
     * osmium buffer with a specified item type.
     */
    class CountHandler : public osmium::handler::Handler
    {
    protected:

        /* Members */

        std::set<osmium::item_type> m_types = {
            osmium::item_type::node,
            osmium::item_type::way,
            osmium::item_type::relation,
            osmium::item_type::area,
            osmium::item_type::changeset,
            osmium::item_type::tag_list,
            osmium::item_type::way_node_list,
            osmium::item_type::relation_member_list,
            osmium::item_type::relation_member_list_with_full_members,
            osmium::item_type::outer_ring,
            osmium::item_type::inner_ring,
            osmium::item_type::changeset_discussion
        };
        
        std::map<osmium::item_type, std::size_t> m_counts = {};
       
        /* Methods */

        void init()
        {
            // Initialize the counts map for each specified type
            for (const osmium::item_type type : m_types)
            {
                m_counts[type] = 0;
            }
        }

    public:

        /* Constructors */

        CountHandler() { init(); };
        CountHandler(osmium::item_type type) : m_types(std::set<osmium::item_type>{ type }) { init(); }
        CountHandler(std::initializer_list<osmium::item_type> types) : m_types(types) { init(); }

        /* Accessors */

        const std::set<osmium::item_type>& types() const
        {
            return m_types;
        }

        const std::size_t& count(osmium::item_type type) const
        {
            return m_counts.at(type);
        }

        const std::map<osmium::item_type, std::size_t>& counts() const
        {
            return m_counts;
        };

        /* Osmium Methods */

        void osm_object(const osmium::OSMObject& object) noexcept
        {
            if (m_types.count(object.type()))
            {
                ++m_counts.at(object.type());
            }
        }

    };

}
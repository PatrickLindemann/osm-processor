#pragma once

#include <algorithm>
#include <osmium/osm/object.hpp>
#include <stdexcept>
#include <string>
#include <tuple>
#include <array>
#include <unordered_set>
#include <unordered_map>

#include "model/memory/types.hpp"
#include "model/memory/node.hpp"
#include "model/memory/way.hpp"
#include "model/memory/relation.hpp"
#include "model/memory/buffer.hpp"

#include <osmium/osm/item_type.hpp>
#include <osmium/osm/relation.hpp>
#include <osmium/osm/types.hpp>
#include <osmium/relations/relations_manager.hpp>

#include <boost/lexical_cast.hpp>

/**
 * Maps an osmium object id to an id in the continous, natural interval [0, N].
 * This bijective mapping ensures that all internal node ids are consecutive,
 * which is not guaranteed for osmium object ids.
 * 
 * @param id The osmium object id
 * @returns The internal object id in the interval [0, N]
 */

using namespace model::memory;

namespace handler
{

    template <typename T>
    class ConvertHandler : public osmium::relations::RelationsManager<ConvertHandler<T>, false, true, false>
    {
    public:

        /**
         * 
         */
        using id_type = object_id_type;

        /**
         * 
         */
        using osm_id_type = osmium::object_id_type;
        
        /**
         * 
         */
        using level_type = unsigned short;

    protected:

        /**
         * 
         */
        std::array<bool, 13> m_filter;
        
        /**
         * 
         */
        Buffer<Node<T>> m_nodes;
        Buffer<Way<T>> m_ways;
        Buffer<Relation<T>> m_relations;

        std::unordered_map<osm_id_type, id_type> m_nids;
        std::unordered_map<osm_id_type, id_type> m_wids;
        std::unordered_map<osm_id_type, id_type> m_rids;

    public:

        /**
         * Construct a Converter.
         * */
        explicit ConvertHandler(
            const level_type territory_level,
            const std::vector<level_type>& bonus_levels
        )
        {
            // Initialize level filter for relations
            m_filter.at(territory_level) = 1;
            for (const auto& level : bonus_levels)
            {
                m_filter.at(level) = 1;
            }
        }

        Buffer<Node<T>> nodes()
        {
            return m_nodes;
        }

        Buffer<Way<T>> ways()
        {
            return m_ways;
        }

        Buffer<Relation<T>> relations()
        {
            return m_relations;
        }

    protected:

        Node<T>& create_node(const osmium::NodeRef& osm_node)
        {   
            // Check if node was inserted already
            auto it = m_nids.find(osm_node.ref());
            if (it != m_nids.end())
            {
                // Node exists, return it
                return m_nodes.get(it->second); 
            }
            // Convert the osmium node and add the result to the node buffer
            id_type mapped_id = m_nids.size();
            m_nids[osm_node.ref()] = mapped_id;
            m_nodes.append({ mapped_id, osm_node.lon(), osm_node.lat() });
            // Return a reference to the newly created node in the container
            return m_nodes.get(mapped_id); 
        }

        Way<T>& create_way(const osmium::Way& osm_way)
        {
            // Check if way was inserted already
            auto it = m_wids.find(osm_way.id());
            if (it != m_wids.end())
            {
                // Way exists, return it
                return m_ways.get(it->second); 
            }
            // Convert the osmium way and add the result to the way buffer
            id_type mapped_id = m_wids.size();
            m_wids[osm_way.id()] = mapped_id;
            m_ways.append({ mapped_id });
            // Add the osmium nodes to the created way and to the node buffer
            Way<T>& way = m_ways.get(mapped_id);
            way.nodes().reserve(osm_way.nodes().size());
            for (const auto& osm_node : osm_way.nodes())
            {
                Node<T>& node = create_node(osm_node);
                way.nodes().push_back(node.id());
            }
            // Return a reference to the newly created way in the container
            return way; 
        }

    public:

        /**
         * We are interested in all relations tagged with type = multipolygon
         * or type = boundary with at least one way member.
         */
        bool new_relation(const osmium::Relation& osm_relation) const
        {
            // Ignore relations without "type" tag
            const char* type = osm_relation.tags().get_value_by_key("type");
            if (type == nullptr)
            {
                return false;
            }
            // Check if relation has a level that was specified to be
            // filtered for.
            const char* admin_level = osm_relation.tags().get_value_by_key("admin_level", "-1");
            short level = boost::lexical_cast<short>(admin_level);
            return level > 0 && m_filter.at(level) == true;
        }

        /**
         * This is called when a relation is complete, ie. all members
         * were found in the input.
         */
        void complete_relation(const osmium::Relation& osm_relation)
        {
            // Convert the osmium relation to an internal relation and add the
            // result to the relation buffer
            id_type mapped_id = m_rids.size();
            m_rids[osm_relation.id()] = mapped_id;
            // Create and add the new area
            Relation<T> relation = { mapped_id };
            // Add the tag values
            relation.add_tag("name", osm_relation.get_value_by_key("name", ""));
            relation.add_tag("type", osm_relation.get_value_by_key("type", ""));
            relation.add_tag("admin_level", osm_relation.get_value_by_key("admin_level", ""));
            // Add the relation's way members
            id_type member_id = 0;
            for (const auto& osm_member : osm_relation.members())
            {
                // Ignore members with invalid references or roles
                if (osm_member.ref() != 0
                 && osm_member.type() == osmium::item_type::way
                 && osm_member.role() != nullptr
                ) {
                    // Retrieve way from osmium object buffer
                    const osmium::Way* osm_way = this->get_member_way(osm_member.ref());
                    // Create the way if it doesn't already exists
                    Way<T>& way = create_way(*osm_way);
                    // Add a reference to the current area depending
                    // on the member role
                    std::string role{ osm_member.role() };
                    if (role == "outer" || role == "inner")
                    {
                        relation.add_member({ member_id, way.id(), Member<T>::Type::WAY, role });
                        ++member_id;
                    }
                    // throw std::invalid_argument("Invalid relation member role");
                }
            }
            m_relations.append(relation);
        }

        void after_way(const osmium::Way& osm_way)
        {
            // yCheck if the way qualifies as an area
            if (osm_way.nodes().size() <= 3
            || !osm_way.nodes().front().location()
            || !osm_way.nodes().back().location()
            || !osm_way.ends_have_same_location()
            || !osm_way.tags().has_tag("area", "no")
            ) {
                return;
            }

            // Check way's admin level
            const char * admin_level = osm_way.get_value_by_key("admin_level", "-1");
            short level = boost::lexical_cast<short>(admin_level);
            if (level < 0 || !m_filter.at(level))
            {
                return;
            }

            // TODO parse way as an area

        }

    };
}
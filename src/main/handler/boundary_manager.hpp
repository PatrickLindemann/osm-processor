#include <set>
#include <string>

#include <osmium/index/id_set.hpp>
#include <osmium/index/nwr_array.hpp>
#include <osmium/index/relations_map.hpp>
#include <osmium/osm/way.hpp>
#include <osmium/tags/tags_filter.hpp>
#include <osmium/relations/relations_manager.hpp>

#include "model/types.hpp"

namespace handler
{

    /**
     * This class collects all data needed for creating areas from
     * relations tagged with type=multipolygon or type=boundary.
     * Most of its functionality is derived from the parent template class
     * osmium::relations::RelationsManager.
     * 
     * This implementation is oriented at existing implementations in the
     * osmium-tool, especially for the tags-filter command:
     * https://github.com/osmcode/osmium-tool/blob/master/src/command_tags_filter.cpp
     */
    class BoundaryManager : public osmium::relations::RelationsManager<BoundaryManager, false, true, false>
    {
    protected:

        /* Types */

        using nwr_array = osmium::nwr_array<osmium::index::IdSetDense<osmium::unsigned_object_id_type>>;

        /* Members */   

        /**
         * 
         */
        osmium::TagsFilter m_filter;

       /**
        *
        */
        nwr_array m_matching_ids;

        /* Methods */

        /**
         * 
         */
        bool is_polygon(const osmium::Way& way) const
        {
            return way.nodes().size() > 3
                && !way.tags().has_tag("area", "no")
                && way.nodes().front().location()
                && way.nodes().back().location()
                && way.ends_have_same_location();
        }

    public:

        /* Constructors */

        BoundaryManager() : m_filter(osmium::TagsFilter{ true }) {}
        BoundaryManager(const osmium::TagsFilter& filter) : m_filter(filter) {}

        /* Accessors */

        const osmium::TagsFilter& filter() const
        {
            return m_filter;
        }

        const nwr_array& matching_ids() const
        {
            return m_matching_ids;
        }

        /* Osmium Methods */

        /**
         * We are interested in all relations tagged with type=multipolygon
         * or type=boundary with at least one way member (and an admin_level
         * contained in the TagFilter if specified)
         */
        bool new_relation(const osmium::Relation& relation) const
        {
            const char* type = relation.tags().get_value_by_key("type");

            // Ignore relations without "type" tag
            if (type == nullptr)
            {
                return false;
            }

            // Check if the relation is an administrative boundary with at least
            // one way.
            if (((!std::strcmp(type, "multipolygon")) || (!std::strcmp(type, "boundary"))) && osmium::tags::match_any_of(relation.tags(), m_filter)) {
                return std::any_of(relation.members().cbegin(), relation.members().cend(), [](const osmium::RelationMember& member) {
                    return member.type() == osmium::item_type::way;
                });
            }

            return false;
        }

        /**
         * This is called when a relation is complete, ie. all members
         * were found in the input.
         * 
         * Note: This method only is called for relations that match the
         * specified filter conditions
         */
        void complete_relation(const osmium::Relation& relation)
        {
            // Mark the relation for insertion
            m_matching_ids(osmium::item_type::relation).set(relation.id());
            const char* admin_level = relation.get_value_by_key(admin_level);
            // Add the way members and their nodes to the output buffer
            for (const auto& member : relation.members())
            {
                if (member.ref() != 0)
                {
                    // We should handle ways only at this point
                    assert(member.type() == osmium::item_type::way);
                    // Mark the way for insertion
                    const osmium::Way* way = this->get_member_way(member.ref());
                    m_matching_ids(osmium::item_type::way).set(way->id());
                    // Mark the referenced nodes for insertion
                    for (const osmium::NodeRef& nr : way->nodes())
                    {
                        if (nr.ref() != 0)
                        {
                            m_matching_ids(osmium::item_type::node).set(nr.positive_ref());
                        }
                    }
                }
            }
        }

        void after_way(const osmium::Way& way)
        {
            // Check if the way describes a valid polygon
            if (!is_polygon(way))
            {
                return;
            }
            // Check that the admin_level of the way is contained in the
            // specified filter
            if (!osmium::tags::match_any_of(way.tags(), m_filter))
            {
                return;
            }
            // Mark the way for insertion
            m_matching_ids(osmium::item_type::way).set(way.id());
            // Mark the referenced nodes for insertion
            for (const osmium::NodeRef& nr : way.nodes())
            {
                if (nr.ref() != 0)
                {
                    m_matching_ids(osmium::item_type::node).set(nr.positive_ref());
                }
            }
        }

    };

}
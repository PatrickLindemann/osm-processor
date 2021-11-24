#pragma once

#include <cstddef>
#include <map>
#include <set>
#include <string>

#include <boost/lexical_cast.hpp>
#include <boost/lexical_cast/bad_lexical_cast.hpp>

#include <osmium/handler.hpp>
#include <osmium/osm/object.hpp>

namespace handler
{

    /**
     * A handler that counts the number of different values
     * for a tag with a specified key.
     */
    template <typename T>
    class TagValueCountHandler : public osmium::handler::Handler
    {
    public:

        /* Types */

        using value_type = T;
        using item_type = osmium::item_type;
        using map_type = std::map<value_type, std::size_t>;

    protected:

        /* Members */

        std::string m_key;
        std::set<item_type> m_types{ item_type::node, item_type::way, item_type::relation };

        std::size_t m_total = 0;
        map_type m_value_counts;

        /* Methods */

        /**
         * Searches for a tag with the specified key in an OSM object
         * and counts its value if it exists.
         * 
         * @param obj The OSM object
         */
        void count_tag(const osmium::OSMObject& obj) noexcept
        {
            // Check object type
            if (!m_types.count(obj.type()))
            {
                return;
            }

            // Retrieve the tag value
            const char * value = obj.get_value_by_key(m_key.c_str());
            if (value != nullptr)
            {
                try
                {
                    // Convert and count the tag value
                    value_type converted_value = boost::lexical_cast<value_type>(value);
                    ++m_total;
                    ++m_value_counts[converted_value];
                }
                catch (boost::bad_lexical_cast& e)
                {
                    // Ignore value
                }
            }
        }

    public:

        /* Constructors */

        TagValueCountHandler(std::string key) : m_key(key) {}
        TagValueCountHandler(std::string key, item_type type) : m_key(key), m_types({ type }) {}
        TagValueCountHandler(std::string key, std::set<item_type> types) : m_key(key), m_types(types) {}

        /* Accessors */

        const std::string& key() const noexcept
        {
            return m_key;
        }

        const std::set<item_type>& types() const noexcept
        {
            return m_types;
        }

        const std::size_t& total() const noexcept
        {
            return m_total;
        }

        const map_type& counts() const noexcept
        {
            return m_value_counts;
        };

        /* Osmium Methods */

        void node(const osmium::Node& node) noexcept
        {
            count_tag(node);
        }

        void way(const osmium::Way& way) noexcept
        {
            count_tag(way);
        }

        void relation(const osmium::Relation& relation) noexcept
        {
            count_tag(relation);
        }

    };

}
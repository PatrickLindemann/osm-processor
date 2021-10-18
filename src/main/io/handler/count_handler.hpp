#pragma once

#include <boost/lexical_cast.hpp>
#include <cstdint>
#include <map>
#include <string>

#include <osmium/osm/node.hpp>
#include <osmium/osm/way.hpp>
#include <osmium/osm/relation.hpp>
#include <osmium/handler.hpp>

#include "util/table.hpp"

namespace io
{

    namespace handler
    {

        /**
         * A handler that counts the total number of nodes, ways and
         * relations of an osmium object stream.
         * 
         * Port of https://github.com/osmcode/libosmium/blob/master/examples/osmium_count.cpp
         */
        class CountHandler : public osmium::handler::Handler {

            /* Members */

            size_t m_nodes     = 0;
            size_t m_ways      = 0;
            size_t m_relations = 0;

        public:

            /* Accessors */

            const size_t& node_count() const
            {
                return m_nodes;
            };

            const size_t& way_count() const
            {
                return m_ways;
            };

            const size_t& relation_count() const
            {
                return m_relations;
            };

            /* Osmium functions */

            void node(const osmium::Node& node) noexcept {
                ++m_nodes;
            }

            void way(const osmium::Way&) noexcept {
                ++m_ways;
            }

            void relation(const osmium::Relation&) noexcept {
                ++m_relations;
            }

        };

        /**
         * A handler that counts the number of different values
         * for a tag with a specified key.
         */
        template <typename T>
        class TagValueCountHandler : public osmium::handler::Handler {
        public:

            /* Types */

            using value_type = T;
            using map_type = std::map<value_type, size_t>;

            /* Members */

            std::string m_key;
            size_t m_total = 0;
            map_type m_value_counts;

        public:

            /* Constructors */

            TagValueCountHandler(std::string key) : m_key(key) {};

            /* Accessors */

            const size_t& total() const
            {
                return m_total;
            }

            const map_type& counts() const
            {
                return m_value_counts;
            };

            /* Osmium methods */

            void relation(const osmium::Relation& relation) noexcept {
                // Retreive tag value
                const char * value = relation.get_value_by_key(m_key.c_str());
                // Check if tag exists
                if (value != nullptr)
                {
                    // Increase counters
                    ++m_total;
                    ++m_value_counts[boost::lexical_cast<value_type>(value)];
                }       
            }

            template <typename StreamType>
            void print(StreamType& stream) const
            {
                util::Table<value_type, size_t> table{
                    { "Value", "Count" },
                    m_value_counts
                };
                table.print(stream);
            }

        };

    }

}
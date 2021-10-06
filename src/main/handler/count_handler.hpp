#pragma once

#include <map>
#include <string>
#include <cstdint>
#include <iostream>

#include <osmium/osm/node.hpp>
#include <osmium/osm/way.hpp>
#include <osmium/osm/relation.hpp>
#include <osmium/handler.hpp>

#include "util/table.hpp"
#include "util/functions.hpp"

namespace handler
{

    /**
     * 
     * Port of https://github.com/osmcode/libosmium/blob/master/examples/osmium_count.cpp
     */
    class CountHandler : public osmium::handler::Handler {
    public:

        using count_type = size_t;

    private:

        /**
         * The counter for node, way and relation objects
         */
        count_type m_nodes     = 0;
        count_type m_ways      = 0;
        count_type m_relations = 0;

    public:

        /* Accessors */

        const count_type node_count() const
        {
            return m_nodes;
        };

        const count_type way_count() const
        {
            return m_ways;
        };

        const count_type relation_count() const
        {
            return m_relations;
        };

        /* Osmium functions */

        void node(const osmium::Node&) noexcept {
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
     * 
     */
    class TagValueCountHandler : public osmium::handler::Handler {
    public:

        using key_type   = std::string;    
        using value_type = std::string;
        using count_type = size_t;
        using map_type   = std::map<value_type, count_type>;

    private:

        /**
         * 
         */
        key_type m_key;

        /**
         * 
         */
        count_type m_total = 0;

        /**
         * 
         */
        map_type m_counts;

    public:

        /* Constructors */

        /**
         * 
         */
        TagValueCountHandler(key_type key) : m_key(key) {};

        /* Accessors */

        const count_type total() const
        {
            return m_total;
        }

        const map_type& counts() const
        {
            return m_counts;
        };

        /* Osmium functions */

        void relation(const osmium::Relation& relation) noexcept {
            // Retreive tag value
            const char * value = relation.get_value_by_key(m_key.c_str());
            // Check if tag exists
            if (value != nullptr)
            {
                // Increase counters
                ++m_total;
                ++m_counts[value_type{ value }];
            }       
        }

        /* Printing functions */

        /**
         * 
         */
        template <typename StreamType>
        void print(StreamType& stream) const
        {
            util::Table<std::string, count_type> table{
                { "Value", "Count" },
                m_counts
            };
            table.print(stream);
        }

    };

}
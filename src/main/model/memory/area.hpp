#pragma once

#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>

#include "model/memory/entity.hpp"
#include "model/memory/ring.hpp"

namespace model
{

    namespace memory
    {

        template <typename T>
        class Area : public Entity
        {
        public:

            /**
             * 
             */
            using ring_type = Ring<T>;

            /**
             * 
             */
            using outers_container = std::vector<ring_type>;

            /**
             * 
             */
            using inners_container = std::vector<ring_type>;


            /**
             * The index map that saves which inner rings belong to which outer
             * rings of the area.
             */
            using index_map_type = std::unordered_map<id_type, std::vector<id_type>>;

            /**
             * The container that stores all referenced ways for this area.
             * This is needed to determine neighborships between areas.
             */
            using reference_container = std::unordered_set<id_type>;

        protected:

            std::string m_name;
            unsigned short m_level;
            outers_container m_outers;
            inners_container m_inners;
            index_map_type m_index_map;
            reference_container m_way_references;

        public:

            Area(id_type id) : Entity(id) {};
            Area(id_type id, std::string name, unsigned short level)
            : Entity(id), m_name(name), m_level(level) {};


            bool empty() const
            {
                return m_outers.empty();
            }

            bool has_exclaves() const
            {
                return m_outers.size() > 1;
            }

            const std::string& name() const
            {
                return m_name;
            }

            const unsigned short& level() const
            {
                return m_level;
            }

            const outers_container& outer_rings() const
            {
                return m_outers;
            }

            const inners_container inner_rings(const ring_type& outer) const
            {
                inners_container result{};
                for (const id_type& id : m_index_map.at(outer.id()))
                {
                    result.push_back(m_inners.at(id));
                }
                return result;
            }

            const reference_container way_references() const
            {
                return m_way_references;
            }

            void add_outer_ring(const ring_type& outer)
            {
                assert(outer.id() == m_outers.size());
                m_outers.push_back(outer);
                m_index_map[outer.id()] = {};
            }

            void add_inner_ring(const ring_type& outer, const ring_type& inner)
            {
                assert(inner.id() == m_inners.size());
                m_inners.push_back(inner);
                m_index_map.at(outer.id()).push_back(inner.id());
            }

            void add_way_reference(const id_type& way_ref)
            {
                m_way_references.insert(way_ref);
            }

        };

    }

}
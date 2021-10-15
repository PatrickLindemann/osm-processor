#pragma once

#include <cassert>
#include <string>
#include <vector>
#include <unordered_map>

#include "model/memory/entity.hpp"
#include "model/memory/ring.hpp"
#include "model/type.hpp"

namespace model
{

    namespace memory
    {

        class OuterRing : public Ring {};
        class InnerRing : public Ring {};

        /**
         * An Area is a non-native OSMObject that is used to
         * describe the geometry of an boundary or multipolygon
         * relation. An area can have multiple outer rings (exclaves)
         * as well as inner rings (holes).
         */
        class Area : public Entity
        {

            /* Types */

            using outers_type = std::vector<OuterRing>;
            using inners_type = std::vector<InnerRing>;
            using ring_map_type = std::unordered_map<object_id_type, std::vector<object_id_type>>;

        protected:

            /**
             * The area name
             */
            std::string m_name;

            /**
             * The area admin_level
             */
            level_type m_level;

            /**
             * The outer ring container.
             */
            outers_type m_outers;

            /**
             * The inner ring container.
             */
            inners_type m_inners;

            /**
             * The index map that saves which inner rings belong to which outer
             * rings of the area.
             */
            ring_map_type m_ring_map;

        public:

            /* Constructors */

            Area(object_id_type id, std::string name, unsigned short level)
            : Entity(id), m_name(name), m_level(level) {};

            /* Accessors */

            const std::string& name() const noexcept
            {
                return m_name;
            }

            const level_type& level() const noexcept
            {
                return m_level;
            }

            const outers_type& outer_rings() const noexcept
            {
                return m_outers;
            }

            /**
             * Access the inner rings of a specified outer ring.
             * 
             * @param outer The outer ring
             * @returns     All inner ring contained in the outer
             *              ring
             */
            const inners_type inner_rings(const OuterRing& outer) const noexcept
            {
                inners_type inners;
                for (const object_id_type& outer_ref : m_ring_map.at(outer.id()))
                {
                    inners.push_back(m_inners.at(outer_ref));
                }
                return inners;
            }

            /* Methods */
            
            /**
             * Check if the area has exclaves, i.e. more than one
             * outer ring.
             */
            bool has_exclaves() const noexcept
            {
                return m_outers.size() > 1;
            }

            /**
             * Add an outer ring to the area.
             * 
             * @param outer The outer ring
             */
            void add_outer(const OuterRing& outer)
            {
                assert(outer.id() == m_outers.size());
                m_outers.push_back(outer);
                m_ring_map[outer.id()] = {};
            }

            /**
             * Insert an inner ring, which is contained in a specified
             * outer ring, to the area.
             * 
             * @param outer The outer ring in which the inner lies in
             * @param inner The inner ring
             */
            void add_inner(const OuterRing& outer, const InnerRing& inner)
            {
                assert(inner.id() == m_inners.size());
                m_inners.push_back(inner);
                m_ring_map.at(outer.id()).push_back(inner.id());
            }

        };
        
        class AreaRef : public EntityRef {};

    }

}
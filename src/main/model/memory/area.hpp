#pragma once

#include <cmath>
#include <vector>
#include <string>
#include <stdexcept>
#include <unordered_map>

#include "model/memory/entity.hpp"
#include "model/memory/node.hpp"
#include "model/memory/ring.hpp"
#include "model/memory/types.hpp"

namespace model
{

    namespace memory
    {

        /**
         * 
         */
        class Area : public Entity
        {
        public:

            /* Types */

            using ring_type      = Ring;
            using ring_list_type = std::vector<ring_type>;
            using ring_map_type  = std::unordered_map<ring_type::id_type, ring_list_type>;
            
        protected:

            /* Members */

            std::string m_name;
            std::string m_type;
            int m_level;
            ring_list_type m_outers;
            ring_map_type m_inners;

        public:

            /* Constructors */

            /**
             * 
             */
            Area(id_type id) : Entity(id) {};

            /**
             * 
             */
            Area(
                id_type id,
                std::string name,
                std::string type,
                int level
            ) : Entity(id), m_name(name), m_type(type), m_level(level) {};

            /* Accessors */

            std::string& name()
            {
                return m_name;
            }

            const std::string& name() const
            {
                return m_name;
            }

            std::string& type()
            {
                return m_type;
            }

            const std::string& type() const
            {
                return m_type;
            }

            int& level()
            {
                return m_level;
            }

            const int& level() const
            {
                return m_level;
            }

            ring_list_type& outer_rings()
            {
                return m_outers;
            }

            const ring_list_type& outer_rings() const
            {
                return m_outers;
            }

            ring_list_type& inner_rings(const ring_type& outer)
            {
                return m_inners.at(outer.id());
            }

            const ring_list_type& inner_rings(const ring_type& outer) const
            {
                return m_inners.at(outer.id());
            }

            /* Setters */

            /**
             * 
             */
            void add_outer(ring_type& outer)
            {
                m_outers.push_back(outer);
                m_inners[outer.id()] = {};
            }

            /**
             * 
             * @throws std::out_of_range
             */
            void add_inner(ring_type& outer, ring_type& inner)
            {
                m_inners.at(outer.id()).push_back(inner);
            }

        };

    }

}
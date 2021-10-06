#pragma once

#include <vector>
#include <algorithm>

#include "model/memory/types.hpp"
#include "model/memory/node.hpp"
#include "model/memory/entity.hpp"

namespace model
{

    namespace memory
    {
        
        /**
         * 
         */
        class Ring : public Entity
        {
        public:

            /* Types */

            using index_type = object_id_type;
            using value_type = EntityRef<Node>;

            using container_type  = EntityRefList<Node>;
            using iterator        = container_type::iterator;
            using const_iterator  = container_type::const_iterator;

        protected:

            /* Members */

            container_type m_nodes;

        public:

            /* Constructors */

            Ring(id_type id) : Entity(id) {};

            Ring(id_type id, container_type nodes) : Entity(id), m_nodes(nodes) {};

            /* Accessors */

            /**
             * 
             */
            container_type& nodes()
            {
                return m_nodes;
            }

            /**
             * 
             */
            const container_type& nodes() const
            {
                return m_nodes;
            }

            /**
             * 
             */
            size_t size() const
            {
                return m_nodes.size();
            }

            /**
             * 
             */
            bool empty() const
            {
                return m_nodes.empty();
            }

            /**
             * 
             */
            void append(value_type value)
            {
                m_nodes.push_back(value);
            }

            /**
             * 
             */
            bool contains(value_type& value)
            {
                return std::find(m_nodes.begin(), m_nodes.end(), value) != m_nodes.end();
            }
            
            /**
             * 
             */
            value_type& get(index_type index)
            {
                return m_nodes.at(index);
            }

            /**
             * 
             */
            const value_type& get(index_type index) const
            {
                return m_nodes.at(index);
            }

            /**
             * 
             */
            value_type& operator[](index_type index)
            {
                return m_nodes[index];
            }
            
            /**
             * 
             */
            const value_type& operator[](index_type index) const
            {
                return m_nodes[index];
            }

            /**
             * 
             */
            void remove(index_type& index)
            {
                m_nodes.erase(m_nodes.begin() + index);
            }
            
            /**
             * 
             */
            iterator begin() noexcept
            {
                return m_nodes.begin();
            }

            /**
             * 
             */
            const_iterator begin() const noexcept
            {
                return m_nodes.cbegin();
            }

            /**
             * 
             */
            iterator end() noexcept
            {
                return m_nodes.end();
            }

            /**
             * 
             */
            const_iterator end() const noexcept
            {
                return m_nodes.cend();
            }

        };

    }

}
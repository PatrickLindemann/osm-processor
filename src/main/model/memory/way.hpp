#pragma once

#include "model/memory/entity.hpp"
#include "model/memory/node.hpp"

namespace model
{

    namespace memory
    {

        template <typename T>
        class Way : public Entity
        {
        public:

            using container_type         = EntityRefList<Node<T>>;
            using iterator               = typename container_type::iterator;
            using const_iterator         = typename container_type::const_iterator;
            using reverse_iterator       = typename container_type::reverse_iterator;
            using const_reverse_iterator = typename container_type::const_reverse_iterator;

        protected:

            container_type m_nodes;

        public:

            Way(id_type id) : Entity(id) {};

            Way(id_type id, container_type& nodes) : Entity(id), m_nodes(nodes) {};
            
            container_type& nodes()
            {
                return m_nodes;
            }

            const container_type& nodes() const
            {
                return m_nodes;
            }

            bool is_closed() const
            {
                return m_nodes.front() == m_nodes.back();
            }

            bool empty() const
            {
                return m_nodes.empty();
            }

            size_t size() const
            {
                return m_nodes.size();
            }

            const EntityRef<Node<T>>& at(size_t index) const
            {
                return m_nodes.at(index);
            }

            const EntityRef<Node<T>>& operator[](size_t index) const
            {
                return m_nodes[index];
            }

            const EntityRef<Node<T>>& front() const
            {
                return m_nodes.front();
            }

            const EntityRef<Node<T>>& back() const
            {
                return m_nodes.back();
            }

            iterator begin() noexcept
            {
                return m_nodes.begin();
            }

            const_iterator cbegin() const noexcept
            {
                return m_nodes.cbegin();
            }

            iterator end() noexcept
            {
                return m_nodes.end();
            }

            const_iterator cend() const noexcept
            {
                return m_nodes.cend();
            }

            reverse_iterator rbegin() noexcept
            {
                return m_nodes.rbegin();
            }

            const_reverse_iterator crbegin() const noexcept
            {
                return m_nodes.crbegin();
            }

            const_reverse_iterator rend() noexcept
            {
                return m_nodes.rend();
            }

            const_reverse_iterator crend() const noexcept
            {
                return m_nodes.crend();
            }

        };

    }
    
}
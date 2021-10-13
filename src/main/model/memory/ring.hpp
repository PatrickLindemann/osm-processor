#pragma once

#include <vector>

#include "model/memory/entity.hpp"

namespace model
{

    namespace memory
    {

        template <typename T>
        class Ring : public Entity
        {
        public:

            using container_type = std::vector<id_type>;
            using iterator = container_type::iterator;
            using const_iterator = container_type::const_iterator;

        protected:

            container_type m_nodes;

        public:

            Ring(id_type id) : Entity(id) {};
            Ring(id_type id, container_type& nodes) : Entity(id), m_nodes(nodes) {};
        
            const bool empty() const
            {
                return m_nodes.empty();
            }

            size_t size() const
            {
                return m_nodes.size();
            }

            void push_back(id_type point)
            {
                m_nodes.push_back(point);
            }

            void pop_back()
            {
                m_nodes.pop_back();
            }

            id_type& at(size_t index)
            {
                return m_nodes.at(index);
            }
            
            const id_type& at(size_t index) const
            {
                return m_nodes.at(index);
            }

            id_type& operator[](const size_t index)
            {
                return m_nodes[index];
            }

            const id_type& operator[](const size_t index) const
            {
                return m_nodes[index];
            }

            iterator begin() noexcept
            {
                return m_nodes.begin();
            }

            const_iterator begin() const noexcept
            {
                return m_nodes.cbegin();
            }

            iterator end() noexcept
            {
                return m_nodes.end();
            }

            const_iterator end() const noexcept
            {
                return m_nodes.cend();
            }

            id_type& front()
            {
                return m_nodes.front();
            }

            const id_type& front() const
            {
                return m_nodes.front();
            }

            id_type& back()
            {
                return m_nodes.back();
            }

            const id_type& back() const
            {
                return m_nodes.back();
            }

        };

    }

}
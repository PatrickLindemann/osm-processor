#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>

#include "model/memory/types.hpp"

namespace model
{
    
    namespace memory
    {

        /**
         * 
         */
        template <typename ValueType>
        class Buffer
        {
        public:

            /* Types */

            using index_type = object_id_type;
            using value_type = ValueType;
            using container_type = std::vector<value_type>;

            using iterator       = typename container_type::iterator;
            using const_iterator = typename container_type::const_iterator;

        private:

            container_type m_container;

        public:

            /* Constructors */

            Buffer() {};

            /**
             * 
             */
            size_t size() const
            {
                return m_container.size();
            }

            /**
             * 
             */
            bool empty() const
            {
                return m_container.empty();
            }

            /**
             * 
             */
            void append(value_type value)
            {
                if (value.id() != m_container.size())
                {
                    throw std::invalid_argument("Value id is not equal to the current position.");
                }
                else
                {
                    m_container.push_back(value);
                }
            };

            /**
             * 
             */
            bool contains(index_type index) const
            {
                return index < m_container.size();
            };
            
            /**
             * 
             */
            value_type& get(index_type index)
            {
                return m_container.at(index);
            };

            /**
             * 
             */
            const value_type& get(index_type index) const
            {
                return m_container.at(index);
            };

            /**
             * 
             */
            value_type& operator[](index_type index)
            {
                return m_container[index];
            }

            /**
             * 
             */
            const value_type& operator[](index_type index) const
            {
                return m_container[index];
            }

            /**
             * 
             */
            iterator begin() noexcept
            {
                return m_container.begin();
            }

            /**
             * 
             */
            const_iterator begin() const noexcept
            {
                return m_container.cbegin();
            }

            /**
             * 
             */
            iterator end() noexcept
            {
                return m_container.end();
            }

            /**
             * 
             */
            const_iterator end() const noexcept
            {
                return m_container.cend();
            }

        };

    }
        
}
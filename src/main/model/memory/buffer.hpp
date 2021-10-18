#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include "model/memory/entity.hpp"
#include "model/type.hpp"

namespace model
{
    
    namespace memory
    {

        /**
         * A Buffer can store a single type of entity objects.
         * 
         * This implementation of a generic reflist inherits
         * the STL vector container. For more information on
         * the available methods, refer to
         * https://en.cppreference.com/w/cpp/container/vector/vector 
         */
        template <typename ValueType>
        class Buffer : private std::vector<ValueType>
        {

            /* Types */

            using value_type = ValueType;

        public:

            /* Constructors */
        
            Buffer() {};

            /* Methods */

             /**
             * Check if the buffer contains a specified entity.
             * 
             * @param value The entity
             * @returns     True if the buffer contains the value
             * 
             * Time complexity: Constant
             */
            bool contains(const value_type& value) const
            {
                return value.id() < this->size();
            };

            /* Methods */
           
            void push_back(value_type value)
            {
                assert(value.id() == this->size());
                std::vector<value_type>::push_back(value);
            };

            value_type& at(object_id_type id)
            {
                return this->at(id);
            }

            const value_type& at(object_id_type id) const
            {
                return this->at(id);
            }

            value_type& at(EntityRef ref)
            {
                return this->at(ref.ref());
            }

            const value_type& at(EntityRef ref) const
            {
                return this->at(ref.ref());
            }

            /* Inherited methods */

            using std::vector<ValueType>::begin;
            using std::vector<ValueType>::end;
            using std::vector<ValueType>::rbegin;
            using std::vector<ValueType>::rend;
            using std::vector<ValueType>::cbegin;
            using std::vector<ValueType>::cend;
            using std::vector<ValueType>::crbegin;
            using std::vector<ValueType>::crend;

            using std::vector<ValueType>::size;
            using std::vector<ValueType>::max_size;
            using std::vector<ValueType>::capacity;
            using std::vector<ValueType>::empty;
            using std::vector<ValueType>::reserve;

            using std::vector<ValueType>::front;
            using std::vector<ValueType>::back;

            using std::vector<ValueType>::pop_back;

        };

    }
        
}
#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include "model/memory/entity.hpp"
#include "model/memory/type.hpp"

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
        class Buffer : public std::vector<ValueType>
        {

            /* Types */

            using value_type = ValueType;

        private:

            // Make some accessor functions private to prevent
            // non-continous insertions
            using std::vector<ValueType>::insert;
            using std::vector<ValueType>::emplace;
            using std::vector<ValueType>::erase;
            using std::vector<ValueType>::emplace_back;
            using std::vector<ValueType>::resize;
            using std::vector<ValueType>::swap;

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

        };

    }
        
}
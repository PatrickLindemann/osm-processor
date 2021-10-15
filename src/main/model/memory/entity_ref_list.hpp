#pragma once

#include <vector>

#include "model/memory/entity.hpp"

namespace model
{

    namespace memory
    {

        /**
         * An EntityRefList is a buffer that contains references to
         * entity objects in memory.
         * 
         * This implementation of a generic reference list inherits
         * the STL vector container. For more information on
         * the available methods, refer to
         * https://en.cppreference.com/w/cpp/container/vector/vector 
         */
        template <typename RefType>
        class EntityRefList : public std::vector<RefType>
        {
        public:

            /* Constructors */

            EntityRefList() : std::vector<RefType>() {};

            /* Methods */

            void push_back(object_id_type ref)
            {
                this->push_back(RefType{ ref });
            }

        };

    }
    
}
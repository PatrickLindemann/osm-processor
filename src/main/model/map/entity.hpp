#pragma once

#include "model/map/type.hpp"

namespace model
{

    namespace map
    {

        class Entity
        {

            /* Members */

            id_type m_id;

        protected:

            /* Constructors */

            Entity(id_type id) : m_id(id) {};

        public:

            const id_type& id() const
            {
                return m_id;
            }

        };

        class EntityRef
        {

            /* Members */

            id_type m_ref;
        
        protected:

            /* Constructors */

            EntityRef(id_type ref) : m_ref(ref) {};

        public:

            const id_type& ref() const
            {
                return m_ref;
            }

        };

    }

}

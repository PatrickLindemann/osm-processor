#pragma once

#include "model/type.hpp"

namespace model
{

    namespace map
    {

        class Boundary
        {

            /* Members */

            object_id_type m_id;

        protected:

            /* Constructors */

            Boundary(object_id_type id) : m_id(id) {};

        public:

            const object_id_type& id() const
            {
                return m_id;
            }

        };

        class BoundaryRef
        {

            /* Members */

            object_id_type m_ref;
        
        protected:

            /* Constructors */

            BoundaryRef(object_id_type ref) : m_ref(ref) {};

        public:

            const object_id_type& ref() const
            {
                return m_ref;
            }

        };

    }

}

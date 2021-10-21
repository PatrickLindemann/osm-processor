#pragma once

#include "model/type.hpp"
#include <ostream>

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
        
        public:

            /* Constructors */

            BoundaryRef(object_id_type ref) : m_ref(ref) {};

            /* Accessors */

            const object_id_type& ref() const
            {
                return m_ref;
            }

        };

        std::ostream& operator<<(std::ostream& out, const BoundaryRef& br)
        {
            return out << br.ref();
        }

    }

}

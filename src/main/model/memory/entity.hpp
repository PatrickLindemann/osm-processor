#pragma once

#include "model/memory/types.hpp"

namespace model
{

    namespace memory
    {

        class Entity
        {
        public:

            /**
             * The type for identifiers.
             * OpenStreetMap contains more than INT_MAX nodes, therefore long is used as identifier type.
             */
            using id_type = object_id_type;

        protected:

            id_type m_id = -1;
            
            /**
             * 
             */
            Entity(id_type id) : m_id(id) {}; /* Can only be instantiated by derived classes */

        public:

            /* Methods */

            /**
             * 
             */
            const id_type id() const
            {
                return m_id;
            }

            /* Operators */

            bool operator==(const Entity& other) const
            {
                return m_id == other.id();
            }

            bool operator!=(const Entity& other) const
            {
                return m_id != other.id();
            }

            bool operator<(const Entity& other) const
            {
                return m_id < other.id();
            }

            bool operator<=(const Entity& other) const
            {
                return m_id <= other.id();
            }

            bool operator>(const Entity& other) const
            {
                return m_id > other.id();
            }

            bool operator>=(const Entity& other) const
            {
                return m_id >= other.id();
            }

        };

    }

}
#pragma once

#include <cstddef>

#include "model/type.hpp"

namespace model
{

    namespace memory
    {

        /**
         * An Entity is a memory object that can be stored
         * in a buffer. Each entity has a positive, unique
         * identifer.
         */
        class Entity
        {

            /* Members */

            object_id_type m_id;
            
        protected:

            /* Constructors */

            Entity(object_id_type id) : m_id(id) {}

        public:

            /* Methods */

            const object_id_type& id() const
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
            
        };

        /**
         * An EntityRef is a reference to an entity
         * object in memory.
         */
        class EntityRef
        {
        
            /* Members */

            object_id_type m_ref;
            
        public:

            /* Constructors */

            EntityRef(object_id_type ref) : m_ref(ref) {}

            /* Methods */

            const object_id_type& ref() const
            {
                return m_ref;
            }

            /* Operators */

            bool operator==(const EntityRef& other) const
            {
                return m_ref == other.ref();
            }

            bool operator==(const object_id_type& id) const
            {
                return m_ref == id;
            }

            bool operator!=(const EntityRef& other) const
            {
                return m_ref != other.ref();
            }

            bool operator!=(const object_id_type& id) const
            {
                return m_ref != id;
            }

        };

        class EntityHasher
        {
        public:
        
            size_t operator()(const Entity& entity) const
            {
                return entity.id();
            }

        };

        class EntityRefHasher
        {
        public:
        
            size_t operator()(const EntityRef& entity_ref) const
            {
                return entity_ref.ref();
            }
            
        };

    }

}
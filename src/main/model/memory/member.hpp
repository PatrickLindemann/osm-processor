#pragma once

#include <string>

#include <boost/algorithm/string/case_conv.hpp>

#include "model/memory/entity.hpp"

namespace model
{

    namespace memory
    {
        
        /**
         * A member is a native OSMObject that is used in ways
         * and relations. Member references can point to nodes,
         * ways and other relations.
         */
        class Member : public EntityRef
        {
        public:

            /* Classes */

            enum Type
            {
                NODE,
                WAY,
                RELATION
            };

        protected:

            /* Members */
            
            /*
            * The type of the object referenced by this member.
            * Members can only reference native OSMObjects, i.e
            * nodes, ways and relations.
            */ 
            Type m_type;

            /**
             * The member role. Each member should have a role, but
             * it is not required.
             */            
            std::string m_role;

        public:

            /* Constructors */

            Member(object_id_type ref) : EntityRef(ref) {};

            Member(object_id_type ref, Type type, std::string role = "")
            : EntityRef(ref), m_type(type), m_role(role)
            {
                boost::algorithm::to_lower(m_role);
            };

            /* Accessors */

            const Type& type() const
            {
                return m_type;
            }

            const std::string& role() const
            {
                return m_role;
            }
        
        };

    }

}
#pragma once

#include <string>

#include <boost/algorithm/string/case_conv.hpp>

#include "model/memory/entity.hpp"
#include "model/memory/types.hpp"

namespace model
{

    namespace memory
    {

        template <typename T>
        class Member : public Entity
        {
        public:

            enum Type
            {
                NODE,
                WAY,
                RELATION
            };

        protected:

            id_type m_ref;
            Type m_type;
            std::string m_role;

        public:

            Member(id_type id) : Entity(id) {};
            Member(id_type id, id_type ref, Type type, const std::string role)
            : Entity(id), m_ref(ref), m_type(type), m_role(role)
            {
                boost::algorithm::to_lower(m_role);
            };

            const id_type& ref() const
            {
                return m_ref;
            }

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
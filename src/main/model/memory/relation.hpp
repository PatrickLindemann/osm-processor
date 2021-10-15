#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include <boost/algorithm/string/case_conv.hpp>

#include "model/memory/entity.hpp"
#include "model/memory/member.hpp"

namespace model
{

    namespace memory
    {

        /**
         * 
         */
        class Relation : public Entity
        {
            
            /* Types */

            using tag_container = std::unordered_map<std::string, std::string>;
            using member_container = std::vector<Member>;

        protected:

            /* Members */

            /**
             * The map of tags as Key-Value pairs.
             */
            tag_container m_tags;

            /**
             * The relation members.
             */
            member_container m_members;

        public:

            /* Constructors */

            Relation(object_id_type id) : Entity(id) {};

            /* Accessors */

            tag_container& tags()
            {
                return m_tags;
            }

            const tag_container& tags() const
            {
                return m_tags;
            }

            member_container& members()
            {
                return m_members;
            }

            const member_container& members() const
            {
                return m_members;
            }

            /* Tag methods */

            void add_tag(std::string key, std::string value)
            {
                m_tags[key] = value;
            }

            const std::string get_tag(std::string key, std::string default_value = "") const
            {
                auto it = m_tags.find(key);
                if (it != m_tags.end())
                {
                    return it->second;
                }
                return default_value;
            }

            /* Member methods */

            void add_member(const Member& member)
            {
                m_members.push_back(member);
            }

            std::vector<Member> get_members_by_role(std::string role) const
            {
                std::vector<Member> members;
                boost::algorithm::to_lower(role);
                for (const auto& member : m_members)
                {
                    if (member.role() == role)
                    {
                        members.push_back(member.ref());
                    }
                }
                return members;
            }

        };

        class RelationRef : public EntityRef {};
        
    }

}
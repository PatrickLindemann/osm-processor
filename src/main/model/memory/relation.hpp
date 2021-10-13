#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include <boost/algorithm/string/case_conv.hpp>

#include "model/memory/entity.hpp"
#include "model/memory/member.hpp"
#include "model/memory/types.hpp"

namespace model
{

    namespace memory
    {

        template <typename T>
        class Relation : public Entity
        {
        public:

            /**
             * 
             */
            using tag_container = std::unordered_map<std::string, std::string>;

            /**
             * 
             */
            using member_container = std::vector<Member<T>>;

        protected:

            tag_container m_tags;
            member_container m_members;

        public:

            Relation(id_type id) : Entity(id) {};
            Relation(id_type id, tag_container& tags)
            : Entity(id), m_tags(tags) {};
            Relation(id_type id, tag_container& tags, member_container& members)
            : Entity(id), m_tags(tags), m_members(members) {};

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

            void add_member(const Member<T>& member)
            {
                m_members.push_back(member);
            }

            std::vector<id_type> get_members_by_role(std::string role) const
            {
                std::vector<id_type> result;
                boost::algorithm::to_lower(role);
                for (const auto& member : m_members)
                {
                    if (member.role() == role)
                    {
                        result.push_back(member.ref());
                    }
                }
                return result;
            }

        };

    }

}
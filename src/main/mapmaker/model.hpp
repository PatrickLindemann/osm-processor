#ifndef MAPMAKER_MODEL_HPP
#define MAPMAKER_MODEL_HPP

#include <any>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "geometry/model.hpp"

namespace mapmaker
{

    namespace model
    {

        /* Boundary */

        struct Boundary
        {
            int64_t id;
            std::string name;
            std::string boundary;
            int32_t level;
            geometry::model::MultiPolygon<double_t> geometry;
            std::string source = "";
            std::string wikidata = "";
        };

        /* Relation */
        
        enum RelationType
        {
            NextTo
        };

        struct Relation
        {
            int64_t left_id;
            int64_t right_id;
            RelationType type;
        };

        /* Map */

        class Map
        {
        public:

            typedef std::unordered_map<int64_t, Boundary> boundaries_type;
            typedef std::unordered_map<int64_t, Relation> relations_type;

        private:

            boundaries_type m_territories;
            boundaries_type m_bonus_links;
            relations_type m_relations;

        public:

            Map() {};
            Map(const std::vector<Boundary>& territories)
            {
                for (const Boundary& t : territories)
                    this->m_territories[t.id] = t;
            };
            Map(const std::vector<Boundary>& territories, const std::vector<Boundary>& bonus_links)
            {
                for (const Boundary& t : territories)
                    this->m_territories[t.id] = t;
                for (const Boundary& b : bonus_links)
                    this->m_bonus_links[b.id] = b;
            };

            /* Accessors */

            const boundaries_type& territories() const
            {
                return m_territories;
            }

            const boundaries_type& bonus_links() const
            {
                return m_bonus_links;
            }

        };

    }

}

#endif
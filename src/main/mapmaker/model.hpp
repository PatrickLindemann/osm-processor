#ifndef MAPMAKER_MODEL_HPP
#define MAPMAKER_MODEL_HPP

#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "geometry/model.hpp"

namespace mapmaker
{

    namespace model
    {

        /* Base class */

        class Entity {};

        enum Type
        {
            eBoundary,
            eMap
        };

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

        /* Map */

        class Map
        {
        public:

            typedef std::vector<Boundary> boundary_array_type;

            Map() {};
            Map(boundary_array_type& boundaries) { this->territories = boundaries; };
            Map(boundary_array_type& boundaries, int width, int height)
            {
                this->territories = boundaries;
                this->width = width;
                this->height = height;
            };

            /* Members */

            int width;

            int height;

            boundary_array_type territories;

            boundary_array_type bonus_links;

            std::unordered_map<int64_t, std::unordered_set<int64_t>> relations;

        };

    }

}

#endif
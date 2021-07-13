#ifndef MAPMAKER_MAP_HPP
#define MAPMAKER_MAP_HPP


#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "model/area.hpp"

namespace mapmaker
{

class Map
{

    int m_width;

    int m_height;

    std::vector<Area> m_territories;

    std::vector<Area> m_bonus_links;

    std::unordered_map<int64_t, std::unordered_set<int64_t>> m_relations;

public:

    Map(std::vector<Area> areas)
    : m_territories(areas)
    {};

    Map(std::vector<Area> areas, int width, int height)
    : m_territories(areas), m_width(width), m_height(height)
    {};

};

}

#endif
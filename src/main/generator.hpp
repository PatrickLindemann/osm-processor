#ifndef MAPMAKER_GENERATOR_HPP
#define MAPMAKER_GENERATOR_HPP

#include <iostream>

#include "model/area.hpp"
#include "model/map.hpp"

namespace mapmaker
{

namespace generator
{

    inline Map run(std::vector<Area>& areas, int width, int height, int territory_level, int bonus_level, double epsilon)
    {
        for (Area& a : areas)
        {
            std::cout << "Area(" << a.id() << "," << a.name() << ")" << std::endl;
        }

        return Map{ areas };
    };
}

}

#endif
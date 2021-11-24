#pragma once

#include <string>
#include <vector>

#include "model/warzone/bonus.hpp"
#include "model/warzone/territory.hpp"

namespace model
{

    namespace warzone
    {
        
        template <typename T>
        struct Map
        {
            std::string name;
            std::size_t width;
            std::size_t height;
            std::set<level_type> levels;
            std::vector<Territory<T>> territories;
            std::vector<Bonus<T>> bonuses;
            std::vector<SuperBonus<T>> super_bonuses;
        };

    }
    
}
#pragma once

#include <vector>

namespace model
{

    namespace memory
    {

        typedef size_t object_id_type;
        
        template <typename EntityType>
        using EntityRef = typename EntityType::id_type;

        template<typename EntityType>
        using EntityList = std::vector<EntityType>;
        
        template<typename EntityType>
        using EntityRefList = std::vector<typename EntityType::id_type>;

    }

}
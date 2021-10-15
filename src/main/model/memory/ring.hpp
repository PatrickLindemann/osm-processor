#pragma once

#include "model/memory/node.hpp"
#include "model/memory/entity_ref_list.hpp"

namespace model
{

    namespace memory
    {
        
        /**
         * A Ring is a non-native OSMObject that describes
         * a closed, non-self-intersecting polyline of nodes.
         * Like ways, rings only store references to nodes rather
         * than the nodes themselves in order to save memory.
         */
        class Ring : public Entity, public EntityRefList<NodeRef>
        {
        public:

            /* Constructors */

            Ring(object_id_type id) : Entity(id), EntityRefList() {};

            /* Methods */

            bool is_closed() const
            {
                return this->front() == this->back();
            }

        };

        class RingRef : public EntityRef {};
        
    }
    
}
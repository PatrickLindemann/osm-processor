#pragma once

#include "model/memory/node.hpp"
#include "model/memory/entity_ref_list.hpp"

namespace model
{

    namespace memory
    {

        /**
         * A Way is a native OSMObject that describes a polyline
         * of nodes. Ways only store references to nodes rather
         * than the nodes themselves in order to save memory.
         */
        class Way : public Entity, public EntityRefList<NodeRef>
        {
        public:

            /* Constructors */

            Way(object_id_type id) : Entity(id), EntityRefList() {};

        };

        class WayRef : public EntityRef {};
        
    }
    
}
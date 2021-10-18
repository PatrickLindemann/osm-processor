#pragma once

#include <bits/stdint-intn.h>

namespace model
{

    /**
     * The type for identifiers, which is the same as the id type used in the
     * libosmium library.
     * OpenStreetMap contains more than 2 billion nodes (as of 2021), which
     * is why the long datatype is needed. While OSM object ids are always
     * positive, in some cases, ids need to be marked as invalid, which is why
     * object_id_type is signed.
     * 
     * For more information, refer to https://osmcode.org/libosmium/manual.html
     */
    using object_id_type = signed long;

    /**
     * 
     */
    using level_type = signed short;

    /**
     * 
     */
    using army_type = signed short;

    /**
     * 
     */
    using component_type = int;

}
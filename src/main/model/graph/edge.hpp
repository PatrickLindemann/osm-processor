#pragma once

#include <utility>

#include "model/graph/vertex.hpp"

namespace model
{

    namespace graph
    {

        /**
         * Edges are defined as pairs of vertices (without a cost metric).
         */
        using edge_type = std::pair<vertex_type, vertex_type>;         

    }

}
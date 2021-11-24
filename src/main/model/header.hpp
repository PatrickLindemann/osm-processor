#pragma once

#include <string>
#include <map>

#include <osmium/io/file_format.hpp>
#include <osmium/io/file_compression.hpp>
#include <osmium/osm/box.hpp>

#include "model/types.hpp"

namespace model
{

    /**
     * A container for OSM file headers and other generic data.
     */
    struct Header
    {
        // File headers
        std::string name;
        osmium::io::file_format format;
        osmium::io::file_compression compression;
        std::size_t size;
        // Osmium object information
        std::size_t nodes;
        std::size_t ways;
        std::size_t relations;
        // Bounding Box information
        osmium::Box bounds;
        // Boundary information
        std::size_t boundaries;
        std::map<level_type, std::size_t> levels;
    };

}
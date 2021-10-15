#pragma once

#include <map>
#include <string>
#include <vector>

#include <boost/filesystem/path.hpp>

#include <osmium/io/file_compression.hpp>
#include <osmium/io/file_format.hpp>
#include <osmium/memory/buffer.hpp>
#include <osmium/osm/types.hpp>

#include "model/geometry/multipolygon.hpp"
#include "model/geometry/rectangle.hpp"
#include "model/geometry/segment.hpp"
#include "model/graph/undirected_graph.hpp"
#include "model/memory/area.hpp"
#include "model/memory/buffer.hpp"
#include "model/memory/node.hpp"
#include "model/memory/relation.hpp"
#include "model/memory/way.hpp"
#include "model/types.hpp"

namespace model
{

    using namespace model::memory;
    using namespace model::geometry;

    /**
     * A container for file metadata and other
     * generic file information.
     */
    struct InfoContainer
    {
        // File information
        std::string name;
        osmium::io::file_format format;
        osmium::io::file_compression compression;
        size_t size;
        
        // Osmium object information
        size_t nodes;
        size_t ways;
        size_t relations;
        
        // Bounding Box information
        Rectangle<double> bounds;

        // Boundary information
        size_t boundary_count;
        std::map<std::string, size_t> level_counts;

        template <typename StreamType>
        void print(StreamType& stream) const
        {
            stream << "File:" << '\n'
                << "  " << "Name: " << name << '\n'
                << "  " << "Format: " << format << '\n'
                << "  " << "Compression: " << compression << '\n'
                << "  " << "Size: " << size << '\n'
                << "Objects:" << '\n'
                << "  " << "Nodes: " << nodes << '\n'
                << "  " << "Ways: " << ways << '\n'
                << "  " << "Relations: " << relations << '\n'
                << "Bounding Box:" << '\n'
                << "  " << "Min: (" << bounds.min.x << ", " << bounds.min.y << ")" << '\n' 
                << "  " << "Max: (" << bounds.max.x << ", " << bounds.max.y << ")" << '\n' 
                << "Boundaries: " << '\n'
                << "  " << "Total: " << boundary_count << '\n'
                << "  " << "Level Distribution: " << '\n';
            for (auto& [level, count] : level_counts)
            {
                stream << "   L" << level << ": " << count << '\n';
            }
            stream << std::endl;
        };

    };

    /**
     * A container for file contents.
     */
    struct DataContainer
    {
        // General information
        std::string name;
        width_type width;
        height_type height;
        level_type territory_level;
        std::vector<level_type> bonus_levels;

        // OSM data buffers
        Buffer<Node> nodes;
        Buffer<Way> ways;
        Buffer<Relation> relations;
        Buffer<Area> areas;

        // Incomplete object buffers
        std::vector<osmium::object_id_type> incomplete_relations;
    };

}
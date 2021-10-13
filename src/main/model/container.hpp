#pragma once

#include <string>
#include <vector>

#include <osmium/memory/buffer.hpp>
#include <osmium/osm/types.hpp>
#include <osmium/io/file_format.hpp>
#include <osmium/io/file_compression.hpp>

#include <boost/filesystem/path.hpp>

#include "model/geometry/rectangle.hpp"
#include "model/memory/node.hpp"
#include "model/memory/types.hpp"
#include "model/memory/way.hpp"
#include "model/memory/relation.hpp"
#include "model/memory/area.hpp"
#include "model/memory/buffer.hpp"
#include "model/graph/undirected_graph.hpp"

namespace model
{

    template <typename T>
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
        geometry::Rectangle<T> bounds { {0, 0}, {0, 0} };

        // Boundary information
        size_t boundaries;
        std::map<std::string, size_t> levels;

        /**
         * @param stream
         */
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
                << "  " << "Total: " << boundaries << '\n'
                << "  " << "Level Distribution: " << '\n';
            for (auto& [level, count] : levels)
            {
                stream << "   L" << level << ": " << count << '\n';
            }
            stream << std::endl;
        };

    };

    /**
     * 
     */
    template<typename T>
    struct DataContainer
    {
        using id_type = memory::object_id_type;
        using component_type = unsigned long;

        // Primitive buffers
        memory::Buffer<memory::Node<T>> nodes;
        memory::Buffer<memory::Way<T>> ways;
        memory::Buffer<memory::Relation<T>> relations;

        // Area buffer
        memory::Buffer<memory::Area<T>> areas;

        // Relation containers
        graph::UndirectedGraph neighbors;
        std::unordered_map<component_type, std::vector<id_type>> components;
        
        // Misc
        std::vector<osmium::object_id_type> incomplete_relations;
    };

}
#pragma once

#include "model/header.hpp"

namespace util
{

    /**
     * Prints the contents of a header container to a specified output stream.
     * 
     * @param stream The output stream
     * @param header The header container
     */
    template <typename StreamType>
    void print(StreamType& stream, const model::Header& header)
    {
        stream << "File:" << '\n'
            << "  " << "Name: " << header.name << '\n'
            << "  " << "Format: " << header.format << '\n'
            << "  " << "Compression: " << header.compression << '\n'
            << "  " << "Size: " << header.size << '\n'
            << "Objects:" << '\n'
            << "  " << "Nodes: " << header.nodes << '\n'
            << "  " << "Ways: " << header.ways << '\n'
            << "  " << "Relations: " << header.relations << '\n'
            << "Bounding Box:" << '\n'
            << "  " << "Min: (" << header.bounds.bottom_left().lon() << ", " << header.bounds.bottom_left().lat() << ")" << '\n' 
            << "  " << "Max: (" << header.bounds.top_right().lon() << ", " << header.bounds.top_right().lat() << ")" << '\n'
            << "Boundaries: " << '\n'
            << "  " << "Total: " << header.boundaries << '\n'
            << "  " << "Level Distribution: " << '\n';
        for (const auto& [level, count] : header.levels)
        {
            stream << "   L" << level << ": " << count << '\n';
        }
        stream << std::endl;
    };

}
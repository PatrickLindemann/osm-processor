#pragma once

#include <osmium/memory/buffer.hpp>
#include <osmium/io/writer.hpp>
#include <osmium/io/any_output.hpp>

#include "io/writer/writer.hpp"

namespace io
{

    /**
     * A reader for API configuration JSON files.
     */
    class BoundaryWriter : public Writer<osmium::memory::Buffer>
    {
    public:

        /* Constructors */

        BoundaryWriter(fs::path file_path) : Writer<osmium::memory::Buffer>(file_path) {}
        
        /* Override Methods */

        void write(osmium::memory::Buffer&& buffer) override
        {
            osmium::io::File file{m_path.string()};
            
            // Create the header and writer
            osmium::io::Header header;
            header.set("generator", "Warzone-OSM-Mapmaker");
            osmium::io::Writer writer{
                file,
                header,
                osmium::io::overwrite::allow, 
                osmium::io::fsync::yes
            };

            // Write the buffer to the output file contents
            writer(std::move(buffer));
            writer.close();
        }

    };

}
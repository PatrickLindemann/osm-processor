#pragma once

#include <osmium/io/file.hpp>
#include <osmium/io/reader.hpp>
#include <osmium/visitor.hpp>

#include "model/types.hpp"
#include "handler/bounds_handler.hpp"
#include "handler/count_handler.hpp"
#include "handler/tag_value_count_handler.hpp"
#include "io/reader/reader.hpp"
#include "model/header.hpp"

namespace io
{

    /**
     * A reader that retrieves general file information for an OSM file.
     */
    class HeaderReader : public Reader<model::Header>
    {
    public:

        /* Constructors */

        HeaderReader(fs::path file_path) : Reader<model::Header>(file_path) {}

        /* Override Methods */

        model::Header read() override
        {
            // The Reader is initialized here with an osmium::io::File, but could
            // also be directly initialized with a file name.
            osmium::io::File file{ m_path.string() };
            osmium::io::Reader reader{file};

            // Create the CountHandler that counts the total number of nodes,
            // ways and relations
            handler::CountHandler count_handler{
                osmium::item_type::node,
                osmium::item_type::way,
                osmium::item_type::relation
            };

            // Create a TagCountHandler that counts the levels for administrative
            // boundaries
            handler::TagValueCountHandler<model::level_type> level_count_handler{
                "admin_level",
                osmium::item_type::relation 
            };

            // Create the BoundsHandler that determines the bounding box of the input
            handler::BoundsHandler bounds_handler;

            // Apply the counters to the input file
            osmium::apply(reader, count_handler, level_count_handler, bounds_handler);

            // You do not have to close the Reader explicitly, but because the
            // destructor can't throw, you will not see any errors otherwise.
            reader.close();

            // Return results
            return model::Header{
                m_path.string(),
                file.format(),
                file.compression(),
                file.buffer_size(),
                count_handler.count(osmium::item_type::node),
                count_handler.count(osmium::item_type::way),
                count_handler.count(osmium::item_type::relation),
                bounds_handler.bounds(),
                level_count_handler.total(),
                level_count_handler.counts()
            };
        }

    };

}
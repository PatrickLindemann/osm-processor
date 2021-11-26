#pragma once

#include "routine.hpp"
#include "io/reader/header_reader.hpp"
#include "model/header.hpp"

#include "util/log.hpp"
#include "util/print.hpp"
#include "util/validate.hpp"

/**
 * The checkout routine retrieves the header (number of objects, bounds, levels,
 * etc.) of an OSM file and prints the results to the console.
 */
class Checkout : public Routine
{

    /* Members */

    /**
     * The path to the input OSM file.
     */
    fs::path m_input;

    /**
    * The logger.
    */
    util::Logger<std::ostream> m_log{ std::cout };

public:

    /* Constructors */

    Checkout() : Routine()
    {
        m_options.add_options()
            ("input", po::value<fs::path>()->required(), "Sets the input file path.\nAllowed file formats: .osm, .pbf")
            ("help,h", "Shows this help message");
        m_positional.add("input", 1);
    }

    /* Override Methods */

    const std::string name() const noexcept override
    {
        return "checkout";
    }

    void setup() override
    {
        Routine::setup();
        this->set<fs::path>(&m_input, "input", util::validate_file);
        m_log.set_steps(1);
    }

    void run() override
    {       
        // Read the file info of the specified input file
        m_log.start() << "Reading headers from file " << m_input << ".\n";
        io::HeaderReader reader{ m_input.string() };
        model::Header header = reader.read();
        m_log.finish();

        util::print(std::cout, header);

        m_log.end();
    }

};
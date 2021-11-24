#pragma once

#include <regex>

#include <osmium/memory/buffer.hpp>

#include "routine.hpp"
#include "io/reader/osm_reader.hpp"
#include "io/writer/osm_writer.hpp"
#include "util/validate.hpp"

/**
 * The prepare routine extracts the boundaries of an OSM file and stores the
 * result for smaller file sizes and faster reading.
 */
class Prepare : public Routine
{

    /* Members */

    /**
     * The path to the input OSM file.
     */
    fs::path m_input;

    /**
     * The output directory for the prepared file.
     */
    fs::path m_outdir;

    /**
    * The output file format.
    */
    std::string m_format;

public:

    /* Constructors */

    Prepare() : Routine()
    {
        m_options.add_options()
            ("input", po::value<fs::path>()->required(), "Sets the input file path.\nAllowed file formats: .osm, .pbf")
            ("outdir,o", po::value<fs::path>()->default_value(""), "Sets the output directory of the prepared boundaries file. If not set, the file will be stored in the executable directory.")
            ("format,f", po::value<std::string>()->default_value("osm.pbf"), "Sets the output format.\n Allowed formats: osm, pbf")
            ("help,h", "Shows this help message");
        m_positional.add("input", 1);
    }

    /* Override Methods */

    const std::string name() const noexcept override
    {
        return "prepare";
    }

    void setup() override
    {
        Routine::setup();
        this->set<fs::path>(&m_input, "input", util::validate_file);
        this->set<fs::path>(&m_outdir, "outdir", m_dir, util::validate_dir);
        this->set<std::string>(&m_format, "format", util::validate_format);
    }

    void run() override
    {
        // Read the boundaries from the specified input file
        io::BoundaryReader reader{m_input};
        osmium::memory::Buffer buffer = reader.read();

        // Prepare the outfile path
        std::string outfile_name = std::regex_replace(
            m_input.filename().string(),
            std::regex("(\\.osm|\\.pbf)"),
            ""
        );
        outfile_name += "-prepared";
        fs::path outfile_path = m_outdir / fs::path(outfile_name).replace_extension(m_format);

        // Write the boundaries to the output
        io::BoundaryWriter writer{outfile_path};
        writer.write(std::move(buffer));
    }   

};
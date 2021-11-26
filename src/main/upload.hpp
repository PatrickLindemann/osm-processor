#pragma once

#include "routine.hpp"

#include "model/config.hpp"

#include "http/mapdata_request.hpp"
#include "http/mapdata_uploader.hpp"
#include "http/response.hpp"
#include "io/reader/config_reader.hpp"
#include "io/reader/mapdata_reader.hpp"

#include "util/log.hpp"
#include "util/validate.hpp"

/**
 * The upload routine uploads map metadata contained in a JSON file to Warzone
 * through the Warzone API.
 * 
 * More informtion on the API can be found under the following link:
 * https://www.warzone.com/wiki/Set_map_details_API
 */
class Upload : public Routine
{

    /* Types */

    using T = double;

    /* Members */

    /**
     * The path to the input metadata JSON file.
     */
    fs::path m_input;

    /**
     * The warzone id of the map.
     */
    long m_id;

    /**
     * The path to the config.json file.
     */
    fs::path m_config_path;

    /**
    * The logger.
    */
    util::Logger<std::ostream> m_log{ std::cout };

public:

    /* Constructors */

    Upload() : Routine()
    {
        m_options.add_options()
            ("input", po::value<fs::path>()->required(), "Sets the input file path.\nAllowed file formats: .json")
            ("map-id", po::value<long>()->required(), "Sets the map id that the metadata changes will be made to")
            ("config,c", po::value<fs::path>()->default_value(""), "Sets the path to the configuration file. If not set, the file will be searched in the executable directory.")
            ("help,h", "Shows this help message.");
        m_positional.add("input", 1);
        m_positional.add("map-id", 1);
        m_log.set_steps(3);
    }

    /* Override Methods */

    const std::string name() const noexcept override
    {
        return "upload";
    }

    void setup() override
    {
        Routine::setup();
        this->set<fs::path>(&m_input, "input", util::validate_file);
        this->set<long>(&m_id, "map-id", util::validate_id);
        this->set<fs::path>(&m_config_path, "config", m_dir / "config.json", util::validate_file);
        m_log.set_steps(3);
    }

    void run() override
    {
        // Read the warzone mapdata file
        m_log.start() << "Reading mapdata from file " << m_input << ".\n";
        io::MapdataReader<T> mapdata_reader{m_input.string()};
        model::warzone::Map<T> map = mapdata_reader.read();
        m_log.finish();

        // Read the config file
        m_log.start() << "Reading configration from " << m_config_path << ".\n";
        io::ConfigReader config_reader{m_config_path.string()};
        model::Config config = config_reader.read();
        m_log.finish();

        // Send the request
        m_log.start() << "Sending request for map " << m_id << " to " << "https://www.warzone.com/API/SetMapDetails" << ".\n";
        http::MapdataRequest<T> request{map, config, m_id};
        http::MapdataUploader<T> uploader{};
        http::Response response = uploader.send(request);
        m_log.step() << "Received response: " << response.code() << " " << response.reason() << '\n'
            << response.body() << ".\n";
        m_log.finish();

        m_log.end();
    }

};
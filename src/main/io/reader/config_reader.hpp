#pragma once

#include <nlohmann/json.hpp>

#include "io/reader/reader.hpp"
#include "model/config.hpp"

namespace io
{

    using json = nlohmann::ordered_json;
    using namespace model;

    /**
     * A reader for API configuration JSON files.
     */
    class ConfigReader : public Reader<model::Config>
    {
    public:

        /* Constructors */

        ConfigReader(fs::path file_path) : Reader<Config>(file_path) {}
        
        /* Override Methods */

        Config read() override
        {
            std::ifstream ifs { m_path };
            json data = json::parse(ifs);           
            return Config{
                data.at("email"),
                data.at("api-token")
            };
        }

    };

}
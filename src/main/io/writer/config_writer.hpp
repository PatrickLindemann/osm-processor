#pragma once

#include <nlohmann/json.hpp>

#include "io/writer/writer.hpp"
#include "model/config.hpp"

namespace io
{

    using json = nlohmann::json;
    using namespace model;

    /**
     * A reader for API configuration JSON files.
     */
    class ConfigWriter : public Writer<Config>
    {
    public:

        /* Constructors */

        ConfigWriter(fs::path file_path) : Writer<Config>(file_path) {}
        
        /* Override Methods */

        void write(Config&& config) override
        {
            std::ofstream ofs{ m_path, std::ios::trunc };
            json data;
            data["email"] = config.email;
            data["api-token"] = config.api_token;
            ofs << data.dump() << std::endl;
        }

    };

}
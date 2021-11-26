#pragma once

#include "io/writer/config_writer.hpp"
#include "model/config.hpp"

#include "util/log.hpp"
#include "util/validate.hpp"

/**
 * The setup routine creates the config.json file with the Warzone user data
 * needed for map uploads through the Warzone API.
 * 
 * More informtion on the API can be found under the following links:
 * https://www.warzone.com/wiki/Set_map_details_API
 * https://www.warzone.com/wiki/Get_API_Token_API
 * https://www.warzone.com/API/GetAPIToken
 */
class Setup : public Routine
{

    /* Members */

    /**
     * The output directory for the generated config.json
     */
    fs::path m_outdir;

    /**
     * The Warzone user e-mail address.
     */
    std::string m_email;

    /**
     * The Warzone user api token.
     */
    std::string m_api_token;

public:

    /* Constructors */

    Setup() : Routine()
    {
        m_options.add_options()
            ("outdir,o", po::value<fs::path>()->default_value(""), "Sets the output directory of the configuration file config.json. If not set, the file will be stored in the executable directory.")
            ("email,e", po::value<std::string>(), "Sets the Warzone user e-mail")
            ("api-token,t", po::value<std::string>(), "Sets the Warzone API Token")
            ("help,h", "Shows this help message.");
    }

    /* Override Methods */

    const std::string name() const noexcept override
    {
        return "setup";
    }

    void setup() override
    {
        Routine::setup();
        this->set<fs::path>(&m_outdir, "outdir", m_dir, util::validate_dir);
        this->set<std::string>(&m_email, "email");
        this->set<std::string>(&m_api_token, "api-token");
    }

    void run() override
    {
        // Prepare the configuration path
        fs::path config_path = m_outdir / "config.json";
        
        // Collect email if it was not entered
        if (m_email.empty())
        {
            std::cout << "Enter your Warzone user e-mail address:" << std::endl;
            std::cin >> m_email;
        }

        // Collect api token if it was not entered
        if (m_api_token.empty())
        {
            std::cout << "Enter your Warzone API token:" << std::endl;
            std::cin >> m_api_token;
        }

        // Write settings to config.json
        model::Config config{m_email, m_api_token};
        io::ConfigWriter writer{config_path};
        writer.write(std::move(config));
        std::cout << "Wrote configuration to " << config_path << "." << std::endl;
    }

};
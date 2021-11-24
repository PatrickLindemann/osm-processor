#pragma once

#include <string>

namespace model
{

    /**
     * A container for the mapmaker api configuration.
     */
    struct Config
    {
        std::string email;
        std::string api_token;
    };

}